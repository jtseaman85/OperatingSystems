// mydu.c - Replicates the behavior of the UNIX os command du

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 255
#endif

/* Special function below is used to process options in the command line args
 *	
 * 		int getopt(int argc, char * const argv[], const char *optstring);
 * 
 * Pass args argc and argv from main and optstring containing option chars you require
 * Returns integer char value of next option found, -1 if end reached
 */

int ShowDirectory(char*, int, int);
int IsDirectory(char*);
int IsLink(char*);
int IsFile(char*);

int main(int argc, char **argv)
{
	int opt;
	char* filepath;
	
	int refLinks = 0;													// Option flags
	int showFiles = 0;
	
	while ((opt = getopt(argc, argv, "aL")) != -1) {					// Process options first
		switch (opt) {
			case 'a':
				showFiles = 1;
				break;
			case 'L':
				refLinks = 1;
				break;
			default:
				break;
		}
	}
	
	filepath = argv[optind];											// optind is a special var from getopt()
	ShowDirectory(filepath,showFiles,refLinks);							// returns index of first non-option arg in argv
	
	return 0;
}

/* ShowDirectory - traverses a file directory and outputs accordingly */
int ShowDirectory(char *param, int aFlag,int lFlag) {
	struct dirent* entry;                                                                       
	struct stat statbuf;                                                                          
	DIR* dir;
    char fullpath[PATH_MAX];                                                                   
   	char chrdir[PATH_MAX];  
	int size = 0;
  
	if ((dir = opendir(param)) == NULL) {
		perror("Failed to open directory");
		return 1;
	}

	while ((entry = readdir(dir))) {
		strcpy(chrdir, "");                                                                     
		strcat(chrdir, param);                                                                  
		strcat(chrdir, "/");                                                                    
		strcat(chrdir, entry->d_name);

		if ((strcmp(entry->d_name, "..") == 0) || (strcmp(entry->d_name, ".") == 0)) {	// if path contains periods, skip
			continue;
		}
		
		if (IsDirectory(chrdir)) {
			size += ShowDirectory(chrdir, aFlag, lFlag);
			
			stat(chrdir, &statbuf);
			printf("%-10d",size);
			printf("\t%s\n", chrdir);		
		} else if (IsFile(chrdir)) {
			stat(chrdir, &statbuf);
			size += (int)statbuf.st_size;
			
			if (aFlag) {
				printf("%-10d",(int)statbuf.st_size);
				printf("\t%s\n", chrdir);
			}
		}
	}
	
	closedir(dir);
	return size;
}

/* IsDirectory - determines if a filepath is a directory */
int IsDirectory(char *path) {                                                                   
	struct stat statbuf;                                                                          
	
	if (stat(path, &statbuf) == -1) {
		return 0;
	} else {
		return S_ISDIR(statbuf.st_mode);
	}
}

/* IsLink - determines if a filepath is a symbolic link */
int IsLink(char *path) {                                                                   
	struct stat statbuf;                                                                          
	
	if (lstat(path, &statbuf) == -1) {
		return 0;
	} else {
		return S_ISLNK(statbuf.st_mode);
	}
}

/* IsFile - determines if a filepath is a file */
int IsFile(char *path) {                                                                       
	struct stat statbuf;                                                                          
	
	if (fstat(open(path,O_RDONLY), &statbuf) == -1) {
		return 0;
	} else {
		return S_ISREG(statbuf.st_mode);
	}
}
