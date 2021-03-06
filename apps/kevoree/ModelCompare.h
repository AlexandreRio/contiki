/*
 * ModelCompare.h
 *
 *  Created on: Apr 29, 2015 6:31:34 PM
 *      Author: Francisco Acosta
 *       eMail: fco.ja.ac@gmail.com
 */

#ifndef MODELCOMPARE_H_
#define MODELCOMPARE_H_

#include <stdbool.h>

typedef struct _TraceSequence TraceSequence;
typedef struct _ContainerRoot ContainerRoot;
typedef enum _Type Type;

TraceSequence *ModelCompare(ContainerRoot *_newModel, ContainerRoot *_currentModel, char *nodeName);
void *actionRemove(char *_path, void *value);
void *actionAdd(char* _path, void *value);
void actionUpdate(char* _path, Type type, void* value);
void actionAddSet(char* _path, Type type, void* value);
void actionprintf(char *path, Type type, void* value);

#endif /* APPS_KEVOREE_RUNTIME_MODELCOMPARE_H_ */
