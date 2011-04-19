
typedef enum jstatus {FOREGROUND, BACKGROUND, STOPPED, DONE, TERMINATED} job_status_t;

typedef struct job_struct {
	char *cmdstring;
	pid_t pgid;
	int job;
	job_status_t jobstat;
	struct job_struct *next;
} joblist_t;

static volatile joblist_t *jobHead;
static volatile joblist_t *jobTail;


void showJobs() {
	joblist_t *listCpy = jobHead;	
	while(listCpy != NULL) {

		fprintf(stderr, "[%d]\t", listCpy->job);

		switch (listCpy->jobstat) {
			case FOREGROUND:
				fprintf(stderr, "FOREGROUND\t");
			case BACKGROUND:
				fprintf(stderr, "BACKGROUND\t");
				break;
			case STOPPED:
				fprintf(stderr, "STOPPED\t");
				break;
			case DONE:
				fprintf(stderr, "DONE\t");
				break;
			case TERMINATED:
				fprintf(stderr, "TERMINATED\t");
				break;
			default:
				break;
		}
		fprintf(stderr, "%ld\t%s", (long)listCpy->pgid, listCpy->cmdstring);
		listCpy = listCpy->next;
	}
	exit(1);
}

int add(pid_t pgid, char *cmd, job_status_t status) {

	joblist_t *job = malloc(sizeof(joblist_t));

	if (status != FOREGROUND && status != BACKGROUND && status != STOPPED) {
		job->cmdstring = malloc(sizeof(char) * (strlen(cmd) + 1));
	
		strcpy(job->cmdstring,cmd);
	
		job->job = getMaxJobNum() + 1;
		job->jobstat = status;
		job->next = NULL;
		
		/* Add the job to the list */
		if (jobHead == NULL) {
			// Handles empty list
			jobHead = job;
			jobTail = jobHead;
		} else {
			jobTail->next = job;
			jobTail = job;
		}
	}
	return 1;
}

int getMaxJobNum() {
	joblist_t *job = jobHead;
	int num = 0;
	
	while(job != NULL) {
		if(job->job > num) {
			num = job->job;
		}
		job = job->next;
	}
	return num;
}
