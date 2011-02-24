// mydu.c - Replicates the behavior of the UNIX os command du

#include <stdio.h>
#include <unistd.h>

/* Special function below is used to process options in the command line args
 *	
 * 		int getopt(int argc, char * const argv[], const char *optstring);
 * 
 * Pass args argc and argv from main and optstring containing option chars you require
 * Returns integer char value of next option found, -1 if end reached
 */

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
	return 0;
}
