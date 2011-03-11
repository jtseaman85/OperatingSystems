#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "restart.h"
#include "restart.c"

// Updated: March 10, 2011

int main (int argc, char *argv[]) {
	pid_t childpid = 0;
	int i, n;
	char reqName[1024];
	char relName[1024];
	
	if (argc != 3) { /* check for valid number of command-line arguments */
		fprintf(stderr, "Usage: %s serverName numClients\n", argv[0]);
		return 1;
	}
	n = atoi(argv[2]);
	
	for (i = 0; i < n; i++)
	{
		if((childpid = fork()) == -1) {
			printf("Failed to fork process.\n");
			return 0;
		}
		
		if (childpid == 0) {
			execl("barrier_client", argv[0], argv[1], (char*)NULL);
			return 0;
		}		
	}
	
	while(wait(NULL) > 0);
	
	strcpy(reqName, argv[1]);
	strcat(reqName, ".request");
	
	strcpy(relName, argv[1]);
	strcat(relName, ".release");
	
	if (unlink(reqName) == -1) {
		perror("Failed to remove FIFO\n");
	}
	
	if (unlink(relName) == -1) {
		perror("Failed to remove FIFO\n");
	}
	
	return 0;
}
