#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "joblist.c"
#define FFLAG (O_WRONLY | O_CREAT | O_TRUNC)
#define FMODE (S_IRUSR | S_IWUSR)
#define BACK_SYMBOL '&'
#define PROMPT_STRING "ush>>"
#define QUIT_STRING "q"

void executecmd(char *incmd);
int makeargv(const char *s, const char *delimiters, char ***argvp);
int parseandredirectin(char *s);
int parseandredirectout(char *s);
int signalsetup(struct sigaction *def, sigset_t *mask, void (*handler)(int));
static sigjmp_buf jumptoprompt;
static volatile sig_atomic_t okaytojump = 0;

/* ARGSUSED */
static void jumphd(int signalnum) {
	if (!okaytojump) return;
	okaytojump = 0;
	siglongjmp(jumptoprompt, 1);
}

int main (void) {
	char *backp;
	sigset_t blockmask;
	pid_t childpid;
	struct sigaction defhandler;
	int inbackground;
	char inbuf[MAX_CANON];
	int len;

	if (signalsetup(&defhandler, &blockmask, jumphd) == -1) {
		perror("Failed to set up shell signal handling");
		return 1;
	}

	for( ; ; ) {
		
		if ((sigsetjmp(jumptoprompt, 1)) && (fputs("\n", stdout) == EOF) ) {	/* if return from signal, newline */
			continue;
		}
		okaytojump = 1;
		//printf("%d",(int)getpid());
		if (fputs(PROMPT_STRING, stdout) == EOF) {
			continue;
		}
		if (fgets(inbuf, MAX_CANON, stdin) == NULL) {
			continue;
		}
		len = strlen(inbuf);
		if (inbuf[len - 1] == '\n') {
			inbuf[len - 1] = 0;
		}
		if (strcmp(inbuf, QUIT_STRING) == 0) {
			break;
		}
		if ((backp = strchr(inbuf, BACK_SYMBOL)) == NULL) {
			inbackground = 0;
		} else {
			inbackground = 1; 
			*backp = 0;
		}
		if (sigprocmask(SIG_BLOCK, &blockmask, NULL) == -1) {
			perror("Failed to block signals");
		}
		if ((childpid = fork()) == -1) {
			perror("Failed to fork");
		} else if (childpid == 0) {
			if (inbackground && (setpgid(0, 0) == -1)) {
				return 1;
			}
			if ((sigaction(SIGINT, &defhandler, NULL) == -1) || (sigaction(SIGQUIT, &defhandler, NULL) == -1) || (sigprocmask(SIG_UNBLOCK, &blockmask, NULL) == -1)) {
				perror("Failed to set signal handling for command "); 
				return 1; 
			}
			executecmd(inbuf);
			return 1;
		}
		if (sigprocmask(SIG_UNBLOCK, &blockmask, NULL) == -1) {
			perror("Failed to unblock signals");
		}
		if (!inbackground) {      /* wait explicitly for the foreground process */
			waitpid(childpid, NULL, 0);
		}
		while (waitpid(-1, NULL, WNOHANG) > 0);    /* wait for background procs */
	}
	return 0;
}

void executecmd(char *incmd) {
	char **chargv;
	if (parseandredirectout(incmd) == -1) {
		perror("Failed to redirect output");
	} else if (parseandredirectin(incmd) == -1) {
		perror("Failed to redirect input");
	} else if (makeargv(incmd, " \t", &chargv) <= 0) {
		fprintf(stderr, "Failed to parse command line\n");
	} else if (strcmp(chargv[0], "jobs") == 0) {
		showJobs();
	} else {
		add(getpgid(), chargv[0], FOREGROUND);
		execvp(chargv[0], chargv);
		perror("Failed to execute command");
	}
	exit(1);
}

int makeargv(const char *s, const char *delimiters, char ***argvp) {
	int error;
	int i;
	int numtokens;
	const char *snew;
	char *t;

	if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
		errno = EINVAL;
		return -1;
	}
	*argvp = NULL;                           
	snew = s + strspn(s, delimiters);         /* snew is real start of string */
	if ((t = malloc(strlen(snew) + 1)) == NULL) {
		return -1; 
	}
	strcpy(t, snew);               
	numtokens = 0;
	if (strtok(t, delimiters) != NULL) {   /* count the number of tokens in s */
		for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ; 
	}
	/* create argument array for ptrs to the tokens */
	if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
		error = errno;
		free(t);
		errno = error;
		return -1; 
	} 
	/* insert pointers to tokens into the argument array */
	if (numtokens == 0) {
		free(t);
	} else {
		strcpy(t, snew);
		**argvp = strtok(t, delimiters);
		for (i = 1; i < numtokens; i++) {
			*((*argvp) + i) = strtok(NULL, delimiters);
		}
	} 
	*((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */
	return numtokens;
}

int parseandredirectin(char *cmd) {    /* redirect standard input if '<' */
	int error;
	int infd;
	char *infile;

	if ((infile = strchr(cmd, '<')) == NULL) {
		return 0;
	}
	*infile = 0;                  /* take everything after '<' out of cmd */
	infile = strtok(infile + 1, " \t");
	if (infile == NULL) {
		return 0;
	}
	if ((infd = open(infile, O_RDONLY)) == -1) {
		return -1;
	}
	if (dup2(infd, STDIN_FILENO) == -1) {
		error = errno;                       /* make sure errno is correct */
		close(infd);
		errno = error;
		return -1;
	}
	return close(infd);
}

int parseandredirectout(char *cmd) {  /* redirect standard output if '>' */
	int error;
	int outfd;
	char *outfile;

	if ((outfile = strchr(cmd, '>')) == NULL) {
		return 0;
	}
	*outfile = 0;                  /* take everything after '>' out of cmd */
	outfile = strtok(outfile + 1, " \t");  
	if (outfile == NULL) {
		return 0;
	}
	if ((outfd = open(outfile, FFLAG, FMODE)) == -1) {
		return -1;
	}
	if (dup2(outfd, STDOUT_FILENO) == -1) {
		error = errno;                        /* make sure errno is correct */
		close(outfd);
		errno = error;
		return -1;
	}
	return close(outfd);
}

int signalsetup(struct sigaction *def, sigset_t *mask, void (*handler)(int)) {
	struct sigaction catch;

	catch.sa_handler = handler;  /* Set up signal structures  */
	def->sa_handler = SIG_DFL;
	catch.sa_flags = 0;    
	def->sa_flags = 0;   
	if ((sigemptyset(&(def->sa_mask)) == -1) || 
			(sigemptyset(&(catch.sa_mask)) == -1) || 
			(sigaddset(&(catch.sa_mask), SIGINT) == -1) || 
			(sigaddset(&(catch.sa_mask), SIGQUIT) == -1) || 
			(sigaction(SIGINT, &catch, NULL) == -1) || 
			(sigaction(SIGQUIT, &catch, NULL) == -1) || 
			(sigemptyset(mask) == -1) || 
			(sigaddset(mask, SIGINT) == -1) || 
			(sigaddset(mask, SIGQUIT) == -1)) {
		return -1;
	}
	return 0;  
}
