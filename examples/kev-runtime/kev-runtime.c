#include "contiki.h"
#include "dev/serial-line.h"
#include "rtkev.h"
#include "cfs/cfs-coffee.h"
#include "loader/elfloader.h"

#include <stdio.h>

const ComponentInterface helloWorld;
const ComponentInterface helloWorld_Second;

DECLARE_KEV_TYPES(2, &helloWorld, &helloWorld_Second)

PROCESS(kevRuntime, "KevRuntime");
AUTOSTART_PROCESSES(&kevRuntime);
PROCESS_THREAD(kevRuntime, ev, data)
{
	static uint8_t buf[257];
	static uint8_t processingFile = 0;
	static uint32_t received = 0;
	static struct cfs_dirent dirent;
	static struct cfs_dir dir;
	static uint32_t fdFile;
	static char *filename;
	PROCESS_BEGIN();

	/* definitively we want to dynamically load modules */
	elfloader_init();
	if (initKevRuntime()) {
		printf("ERROR: Model cannot be loaded\n");
		printf("Runtime initialization error\n");
		PROCESS_EXIT();
	}
	/* let's register core components */
	REGISTER_KEV_TYPES_NOW();

	printf("Kevoree server started !\n");
	while(1) {

		PROCESS_YIELD();
		if (ev == serial_line_event_message) {
			if (!strcmp(data, "ls")) {
				if(cfs_opendir(&dir, ".") == 0) {
				  while(cfs_readdir(&dir, &dirent) != -1) {
					printf("File: %s (%ld bytes)\n",
						dirent.name, (long)dirent.size);
				  }
				  cfs_closedir(&dir);
				}
			}
			else if (strstr(data, "createInstance") == (int)data) {
				printf("Executing createInstance\n");
				char* tmp = strstr(data, " ");
        		tmp++;
				char* tmp2 = strstr(tmp, " ");
				*tmp2 = 0;
				printf("\tParam 0 : %s\n", tmp);
        		tmp2++;
				printf("\tParam 1 : %s\n", tmp2);
				void* ins;
				createInstance(tmp, tmp2, &ins);
				printf("\tThe created instance has adress %p\n", ins);	
			}
			else if (!strcmp(data, "format")) {
				/* format the flash */
				printf("Formatting\n");
				printf("It takes around 3 minutes\n");
				printf("...\n");

				fdFile = cfs_coffee_format();
				printf("Formatted with result %ld\n", fdFile);
			}
			else if (strstr(data, "cat") == (int)data) {
				int n, jj;
				char* tmp = strstr(data, " ");
				tmp++;
				fdFile = cfs_open(tmp, CFS_READ);
				if (fdFile < 0) printf("error opening the file %s\n", tmp);
				while ((n = cfs_read(fdFile, buf, 60)) > 0) {
				  for (jj = 0 ; jj < n ; jj++) printf("%c", (char)buf[jj]);
				}
				printf("\n");
				cfs_close(fdFile);
				if (n!=0)
					printf("Some error reading the file\n");
			}
			else if (strstr(data, "rm") == (int)data) {
				int n, jj;
				char* tmp = strstr(data, " ");
				tmp++;
				cfs_remove(tmp);
			}
			else if (strstr(data, "loadelf") == (int)data) {
				filename = strstr(data, " ");
				filename++;
				// Cleanup previous loads
				if (elfloader_autostart_processes != NULL)
					autostart_exit(elfloader_autostart_processes);
				elfloader_autostart_processes = NULL;

				// Load elf file
				fdFile = cfs_open(filename, CFS_READ | CFS_WRITE);
				received = elfloader_load(fdFile);
				cfs_close(fdFile);
				printf("Result of loading %lu\n", received);

				// As the file has been modified and can't be reloaded, remove it
				printf("Remove dirty firmware '%s'\n", filename);
				cfs_remove(filename);

				// execute the program
				if (ELFLOADER_OK == received) {
					if (elfloader_autostart_processes) {
						//PRINT_PROCESSES(elfloader_autostart_processes);
						autostart_start(elfloader_autostart_processes);
					}
				}
				else if (ELFLOADER_SYMBOL_NOT_FOUND == received) {
				  printf("Symbol not found: '%s'\n", elfloader_unknown);
				}
			}
			else if (strstr(data, "upload") == (int)data) {
				char* tmp = strstr(data, " ");
				tmp++;
				fdFile = cfs_open(tmp, CFS_READ | CFS_WRITE);
				printf("Uploading file %s\n", tmp);
				processingFile = 1;
			}
			else if (!strcmp(data, "endupload")) {
				cfs_close(fdFile);
				printf("File uploaded (%ld bytes)\n", received);
				received = 0;
				processingFile = 0;
			}
			else if (processingFile) {
				int n = strlen(data);
				int r = decode(data, n, buf);
				received += r;
				cfs_write(fdFile, buf, r);
			}
			else  {
				printf("%s (%lu bytes received)\n", (char*)data, received);
			}
		}
	}

	PROCESS_END();
}