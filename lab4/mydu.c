// mydu.c - Replicates the behavior of the UNIX os command du

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

/* Special function below is used to process options in the command line args
 *	
 * 		int getopt(int argc, char * const argv[], const char *optstring);
 * 
 * Pass args argc and argv from main and optstring containing option chars you require
 * Returns integer char value of next option found, -1 if end reached
 */

void TraverseDir(char*, int, int);
int IsDirectory(char*);
int IsLink(char*);
int IsFile(char*);

int main(int argc, char **argv)
{
	int opt;
	
	// Option flags
	int refLinks = 0;
	int expandDirs = 0;
	
	// Process options first
	while ((opt = getopt(argc, argv, "aL")) != -1) {
		switch (opt) {
			case 'a':
				expandDirs = 1;
				break;
			case 'L':
				refLinks = 1;
				break;
			default:
				break;
		}
	}
	
	// For now, just output results with no options
	TraverseDir("./home/jtseaman",0,0);
	
	return 0;
}

void TraverseDir(char *filePath, int aFlag,int lFlag) {                                                                                                                                    

   	struct dirent *dirEntPtr;
   	struct stat statBuffer;                                                                          
   	DIR *dirPtr;
                                                                     
   	char currPath[1000];  
	int size=0;
  
	dirPtr = opendir(filePath);
	
	
	
	closedir(dirPtr);
	return;
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
