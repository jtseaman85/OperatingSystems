#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	pid_t childpid;             /* indicates process should spawn another     */
	int error;                  /* return value from dup2 call                */
	int fd[2];                  /* file descriptors returned by pipe          */
	int i;                      /* number of this process (starting with 1)   */
	int nprocs;                 /* total number of processes in ring          */ 
	char str[4] = "1 1";
	char* strcopy;
	char* val;
	int myfd;
	int isLast = 0;
	int x = 0;
	int y = 0;
	
	/* check command line for a valid number of processes to generate */
	if ( (argc != 2) || ((nprocs = atoi (argv[1])) <= 0) ) {
		fprintf (stderr, "Usage: %s nprocs\n", argv[0]);
		return 1; 
	}
	
	if (pipe(fd) == -1) {      /* connect std input to std output via a pipe */
		perror("Failed to create starting pipe");
		return 1;
	}
	
	if ((dup2(fd[0], STDIN_FILENO) == -1) || (dup2(fd[1], STDOUT_FILENO) == -1)) {
		perror("Failed to connect pipe");
		return 1;
	}
	
	if (write(fd[1], str, sizeof(str)) == -1) {
		fprintf(stderr, "[%ld]:failed to write to pipe: %s\n", (long)getpid(), strerror(errno));
		return 1;
	}
	
	if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
		perror("Failed to close extra descriptors");
		return 1; 
	} 
      
	for (i = 1; i < nprocs;  i++) {         /* create the remaining processes */
		if (pipe(fd) == -1) {
			fprintf(stderr, "[%ld]:failed to create pipe %d: %s\n", (long)getpid(), i, strerror(errno));
			return 1; 
		} 
	
		if ((childpid = fork()) == -1) {
			fprintf(stderr, "[%ld]:failed to create child %d: %s\n", (long)getpid(), i, strerror(errno));
			return 1; 
		} 
	
		if (childpid > 0) {             /* for parent process, reassign stdout */
			error = dup2(fd[1], STDOUT_FILENO);
			
			if (write(fd[1], str, sizeof(str)) == -1) {
				fprintf(stderr, "[%ld]:failed to write to pipe: %s\n", (long)getpid(), strerror(errno));
				return 1;
			}
		} else {                        /* for child process, reassign stdin */
			error = dup2(fd[0], STDIN_FILENO);
			
			if (read(fd[0], str, sizeof(str)) == -1) {
				fprintf(stderr, "[%ld]:failed to read pipe: %s\n", (long)getpid(), strerror(errno));
				return 1;
			}
			
			strcopy = (char*) malloc(strlen(str)+1);
			strcpy(strcopy, str);
			
			strcopy = strtok(strcopy, " ");
			x = atoi(strcopy);
			
			strcopy = strtok(NULL, "");
			y = atoi(strcopy);
			
			x = x + y;
			y = x + y;
			
			sprintf(str, "%d %d", x, y);
			
			free(strcopy);
		}
		if (error == -1) {
			fprintf(stderr, "[%ld]:failed to dup pipes for iteration %d: %s\n", (long)getpid(), i, strerror(errno));
			return 1; 
		}
		
		if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
			fprintf(stderr, "[%ld]:failed to close extra descriptors %d: %s\n", (long)getpid(), i, strerror(errno));
			return 1; 
		} 
	
		if (i == (nprocs - 1) && !childpid) {
			isLast = 1;
		}
	
		if (childpid)
			break;
	}


	if (isLast) {
		fprintf(stderr, " %s \n", str);
	} else {
		fprintf(stderr, " %s ", str);
	}
	
	while(wait(NULL) > 0);
	return 0; 
}
