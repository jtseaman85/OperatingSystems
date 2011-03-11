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
	char testChar[] = "JOHN";
	char tosend[1];
	char toreceive[1];
	int requestfd;
	int releasefd;
	int i;
	char reqName[1024];
	char relName[1024];

	if (argc != 2) {  /* name of server fifo is passed on the command line */
		fprintf(stderr, "Usage: %s fifoname\n", argv[0]);
		return 1; 
	}
	
	strcpy(reqName, argv[1]);
	strcat(reqName, ".request");
	
	strcpy(relName, argv[1]);
	strcat(relName, ".release");
	
	// server loops twice 
	for (i = 0; i < 2; i++) {
		tosend[0] = testChar[i];
		
		fprintf(stderr, "Client %d Iteration #%d\n", getpid(), i + 1);
	
		// open a read/write communication endpoint to the pipe
		fprintf(stderr, "Client %d opening %s\n", getpid(), reqName); 
		if ((requestfd = open(reqName, O_WRONLY)) == -1) {
			perror("Client failed to open its request FIFO\n");
			return 1; 
		}
		
		fprintf(stderr, "Client %d writing '%c' to %s\n", getpid(), testChar[i], reqName); 

		if (write(requestfd, tosend, sizeof(tosend)) < 0) {
			perror("Client failed to write character\n");
			return 1;
		}
		
		fprintf(stderr, "Client %d wrote successfully to pipe\n", getpid());
		
		fprintf(stderr, "Client %d closing %s\n", getpid(), reqName);
		close(requestfd);

		fprintf(stderr, "Client %d opening %s\n", getpid(), relName);
		if ((releasefd = open(relName, O_RDONLY)) == -1) {
			perror("Client failed to open its release FIFO\n");
			return 1; 
		}
		
		fprintf(stderr, "Client %d reading from %s\n", getpid(), relName); 

		if (read(releasefd, toreceive, sizeof(toreceive)) < 0) {
			perror("Sever failed to read character\n");
			return 1;
		}

		fprintf(stderr, "Client %d successfully read '%c' from pipe\n", getpid(), toreceive[0]);
		
		fprintf(stderr, "Client %d closing %s\n", getpid(), relName);
		close(releasefd);
	}	

	return 0; 
}

