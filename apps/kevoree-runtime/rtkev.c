#include "rtkev.h"
#include "lib/list.h"

#include "ContainerRoot.h"
#include "JSONModelLoader.h"

#include <stdarg.h>

#define DEBUG
#ifdef DEBUG
#define PRINTF printf
#else
#define PRINTF(S, ...)
#endif

/**
 * The Kevoree runtime for contiki includes a set of processes
 * that perform different tasks.
 *
 * Of course, it also includes several datastructure to represent the internal runtime
 * state.
 *
 */

struct TypeEntry {
	struct TypeEntry* next;
	ComponentInterface* interface;
};

struct InstanceEntry {
	struct InstanceEntry* next;
	void* instance;
	char* name;
};

static struct Runtime {
    /* the current model */
	ContainerRoot *currentModel;
    /* hash_map from type name to type definition */
	LIST_STRUCT(types);
	/* instanaces */
	LIST_STRUCT(instances);
} runtime;

static const char *DEFAULTMODEL = "{\"eClass\" : \"org.kevoree.ContainerRoot\",\"generated_KMF_ID\" : \"BXX5q3eV\",\"nodes\" : [{\"eClass\" : \"org.kevoree.ContainerNode\",\"name\" : \"node0\",\"metaData\" : \"\",\"started\" : \"1\",\"components\" : [],\"hosts\" : [],\"host\" : [],\"groups\" : [\"groups[group0]\"],\"networkInformation\" : [{\"eClass\" : \"org.kevoree.NetworkInfo\",\"name\" : \"ip\",\"values\" : [{\"eClass\" : \"org.kevoree.NetworkProperty\",\"name\" : \"front\",\"value\" : \"m3-XX.lille.iotlab.info\"},{\"eClass\" : \"org.kevoree.NetworkProperty\",\"name\" : \"local\",\"value\" : \"fe80:0000:0000:0000:0323:4501:4471:0343\"}]}],\"typeDefinition\" : [\"typeDefinitions[ContikiNode/0.0.1]\"],\"dictionary\" : [],\"fragmentDictionary\" : []}],\"typeDefinitions\" : [{\"eClass\" : \"org.kevoree.NodeType\",\"name\" : \"ContikiNode\",\"version\" : \"0.0.1\",\"factoryBean\" : \"\",\"bean\" : \"\",\"abstract\" : \"0\",\"deployUnit\" : [\"deployUnits[org.kevoree.library.c//kevoree-contiki-node/0.0.1]\"],\"dictionaryType\" : [{\"eClass\" : \"org.kevoree.DictionaryType\",\"generated_KMF_ID\" : \"o8AVQY3e\",\"attributes\" : []}],\"superTypes\" : []},{\"eClass\" : \"org.kevoree.GroupType\",\"name\" : \"CoAPGroup\",\"version\" : \"0.0.1\",\"factoryBean\" : \"\",\"bean\" : \"\",\"abstract\" : \"0\",\"deployUnit\" : [\"deployUnits[//kevoree-group-coap/0.0.1]\"],\"dictionaryType\" : [{\"eClass\" : \"org.kevoree.DictionaryType\",\"generated_KMF_ID\" : \"3dddTFpd\",\"attributes\" : [{\"eClass\" : \"org.kevoree.DictionaryAttribute\",\"name\" : \"proxy_port\",\"optional\" : \"1\",\"state\" : \"0\",\"datatype\" : \"int\",\"fragmentDependant\" : \"1\",\"defaultValue\" : \"20000\"},{\"eClass\" : \"org.kevoree.DictionaryAttribute\",\"name\" : \"port\",\"optional\" : \"1\",\"state\" : \"0\",\"datatype\" : \"number\",\"fragmentDependant\" : \"1\",\"defaultValue\" : \"\"},{\"eClass\" : \"org.kevoree.DictionaryAttribute\",\"name\" : \"path\",\"optional\" : \"1\",\"state\" : \"0\",\"datatype\" : \"string\",\"fragmentDependant\" : \"1\",\"defaultValue\" : \"\"}]}],\"superTypes\" : []}],\"repositories\" : [],\"dataTypes\" : [],\"libraries\" : [{\"eClass\" : \"org.kevoree.TypeLibrary\",\"name\" : \"ContikiLib\",\"subTypes\" : [\"typeDefinitions[ContikiNode/0.0.1]\",\"typeDefinitions[CoAPGroup/0.0.1]\"]},{\"eClass\" : \"org.kevoree.TypeLibrary\",\"name\" : \"Default\",\"subTypes\" : []}],\"hubs\" : [],\"mBindings\" : [],\"deployUnits\" : [{\"eClass\" : \"org.kevoree.DeployUnit\",\"name\" : \"kevoree-group-coap\",\"groupName\" : \"\",\"version\" : \"0.0.1\",\"url\" : \"\",\"hashcode\" : \"\",\"type\" : \"ce\"},{\"eClass\" : \"org.kevoree.DeployUnit\",\"name\" : \"kevoree-contiki-node\",\"groupName\" : \"org.kevoree.library.c\",\"version\" : \"0.0.1\",\"url\" : \"\",\"hashcode\" : \"\",\"type\" : \"ce\"}],\"nodeNetworks\" : [],\"groups\" : [{\"eClass\" : \"org.kevoree.Group\",\"name\" : \"group0\",\"metaData\" : \"\",\"started\" : \"1\",\"subNodes\" : [\"nodes[node0]\"],\"typeDefinition\" : [\"typeDefinitions[CoAPGroup/0.0.1]\"],\"dictionary\" : [],\"fragmentDictionary\" : [{\"eClass\" : \"org.kevoree.FragmentDictionary\",\"generated_KMF_ID\" : \"VEj2RlNr\",\"name\" : \"contiki-node\",\"values\" : []}]}]}";

/* kevoree event types */
static process_event_t NEW_KEV_TYPE;
static process_event_t NEW_MODEL;

/* internal structures used to send data along events */
typedef struct {
	void* first;
	void* second;
} Pair;

PROCESS(kev_model_listener, "kev_model_listener");
PROCESS_THREAD(kev_model_listener, ev, data)
{
    PROCESS_BEGIN();

	PRINTF("Ejecutando proceso kev_model_listener\n");

	/* register new event type */
	NEW_MODEL = process_alloc_event();

    while (1) {
        /* it runs forever, waiting for some update to the model */
		PROCESS_WAIT_EVENT();
		if (ev == NEW_MODEL) {

			/* wow I ave a new model, do te magic with the traces and so on */
			PRINTF("Here a new model is coming\n");
		
		}
    }

    PROCESS_END();
}

int initKevRuntime()
{
	LIST_STRUCT_INIT(&runtime, types);
	LIST_STRUCT_INIT(&runtime, instances);

	/* let's assign the empty model as the current model */
	struct jsonparse_state jsonState;

	jsonparse_setup(&jsonState, DEFAULTMODEL, strlen(DEFAULTMODEL) + 1);
	runtime.currentModel = JSONKevDeserializer(&jsonState, jsonparse_next(&jsonState));

	if (runtime.currentModel == NULL)
		return ERR_KEV_INIT_FAILURE;

	/* start support protothreads */
	process_start(&kev_model_listener, NULL);

	return 0;
}

/* register component type */
int registerComponent(int count, ... )
{
	va_list ap;
	ComponentInterface* interface;
	
	/* this is here for debug, when you deploy an example which is a component 
	 * you must start the runtime somehow	
	*/	
	/*initKevRuntime();*/

	/* get the arguments */
	va_start(ap, count);

	/* iterate to register arguments */
	while (count) {
		interface = va_arg(ap, ComponentInterface*);
		count--;

		PRINTF("En registrar componente %s %p\n", interface->name, interface);
		
		/* it add a new entry to the list :-) */
		struct TypeEntry* entry = (struct TypeEntry*)malloc(sizeof(struct TypeEntry));
		entry->interface = interface;

		list_add(runtime.types, entry);
	}

	/* done with the variadic arguments*/
	va_end(ap);

	return 0; 
}

/* Notify about a new model, normally this will be mostly used Groups.
 * However, it is also available for "smart"components.
 * it receive a ContainerRoot as parameter
 * */
int notifyNewModel(ContainerRoot *model)
{
	// it essentially sends a message to the process kev_model_listener
	// well, I am guessing everything is Ok, :-)
	return process_post(&kev_model_listener, NEW_MODEL, model);
}

/* create an instance of some type */
int createInstance(char* typeName, char* instanceName, void** instance)
{
	struct TypeEntry* entry;
	/* iterate through list of ComponentInterface */
	for(entry = list_head(runtime.types);
      entry != NULL;
      entry = list_item_next(entry)) {
		if (!strcmp(typeName, entry->interface->name)) {
			PRINTF("\tType Found\n");
			
			/* create instance using supplied component interface */
			*instance = entry->interface->newInstance(entry->interface->name);

			if (!*instance)
				return ERR_KEV_INSTANCE_CREATION_FAIL;
			
			struct InstanceEntry* entry = (struct InstanceEntry*)malloc(sizeof(struct InstanceEntry));
			list_add(runtime.instances, entry);
		}
	}
	return 0;
}