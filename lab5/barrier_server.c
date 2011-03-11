#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "restart.h"
#include "restart.c"
#define FIFO_PERMS (S_IRWXU | S_IWGRP| S_IWOTH)

// Updated: March 10, 2011

int main (int argc, char *argv[]) {
	FILE* pipe = NULL;
	char reqName[1024];
	char relName[1024];
	int i;
	int requestfd;
	int releasefd;
	int x;
	int n = atoi(argv[2]);
	char buffer[n];
	
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [barrier name] [barrier size]\n", argv[0]);
		return 1; 
	}
	
	strcpy(reqName, argv[1]);
	strcat(reqName, ".request");
	
	strcpy(relName, argv[1]);
	strcat(relName, ".release");
	
	// create a named pipe to handle incoming requests
	if ((mkfifo(reqName, FIFO_PERMS) == -1) && (errno != EEXIST)) {
		perror("Server failed to create a FIFO\n");
		return 1; 
	}
	
	// create a named pipe to handle writing requests
	if ((mkfifo(relName, FIFO_PERMS) == -1) && (errno != EEXIST)) {
		perror("Server failed to create a FIFO\n");
		return 1;
	}
	
	// server loops twice 
	for (i = 0; i < 2; i++) {
		fprintf(stderr, "Iteration #%d\n", i + 1);
	
		// open a read/write communication endpoint to the pipe
		fprintf(stderr, "Server opening %s\n", reqName); 
		if ((requestfd = open(reqName, O_RDONLY)) == -1) {
			perror("Server failed to open its request FIFO");
			return 1;
		}
		
		fprintf(stderr, "Server reading from %s\n", reqName); 

		if (read(requestfd, buffer, sizeof(buffer)) < 0) {
			perror("Sever failed to read character\n");
			return 1;
		}

		fprintf(stderr, "Server read successfully from pipe\n");
		
		fprintf(stderr, "Server closing %s\n", reqName);
		close(requestfd);
		
		pipe = NULL;
		
		fprintf(stderr, "Server opening %s\n", relName);
		if ((releasefd = open(relName, O_WRONLY)) == -1) {
			perror("Server failed to open its release FIFO\n");
			return 1;
		}
		
		fprintf(stderr, "Server writing to %s\n", relName); 
		
		if (write(releasefd, buffer, sizeof(buffer)) < 0) {
			perror("Sever failed to read character\n");
			return 1;
		}
		
		fprintf(stderr, "Server wrote successfully to pipe\n");
		
		fprintf(stderr, "Server closing %s\n", relName);
		close(releasefd);
		
		pipe = NULL;
	}
	
	fprintf(stderr, "Server closing\n");
	return 0; 
}
