#include "mush.h"

/********************* Pipes *********************/

/*initializes a pipeArr*/
void initPipeArr(pipeArr *pa) {
	memset(pa, sizeof(*pa), 0);
}

/*opens all possible pipes*/
void openPipes(pipeArr *pa) {
	int i;
	for (i = 0; i < MAX_CMD_PIPES - 1; i++) {
		if (pipe(pa->pipes[i]) < 0)
			perror(NULL);
	}
}

/*close all the pipes*/
void closeAllPipes(pipeArr *pa){
	int i, max = MAX_CMD_PIPES - 2;
	for(i=0; i < max; i++){
		if(close(pa->pipes[i][RD_END]) || close(pa->pipes[i][WR_END]))
			perror(NULL);
	}
}

/********************* main functions *********************/

/*forks and executes the given pipeline of processes*/
void execProcesses(fileSet *fs, pipeArr *pa) {
	int i, fdin, fdout;
	int error = 0; /*set to 1 if theres a probem and running processes need to be killed*/
	cmdFile *cf;
	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

	openPipes(pa);

	for (i = 0; i < fs->size; i++) {
		cf = fs->files + i;

		if (!(cf->pid = fork())) { /*in child*/
			/*set up file input*/
			if (cf->inStage != -1)
				dup2(pa->pipes[i - 1][RD_END], STDIN_FILENO);
			else if (cf->inName) {
				if ((fdin = open(cf->inName, O_RDONLY)) < 0) {
					perror(cf->inName);
					error = 1;
				} else {
					dup2(fdin, STDIN_FILENO);
					close(fdin);
				}
			}

			/*set up file output*/
			if (cf->outStage != -1)
				dup2(pa->pipes[i][WR_END], STDOUT_FILENO);
			else if (cf->outName) {
				if ((fdout = open(cf->outName, O_WRONLY | O_CREAT | O_TRUNC, mode)) < 0) {
					perror(cf->outName);
					error = 1;
				} else {
					dup2(fdout, STDOUT_FILENO);
					close(fdout);
				}
			}

			closeAllPipes(pa);
			sigaction(SIGINT, &old_sa, NULL); /*Restore interrupts for the child*/
			
			/*exec if no errors thus far*/
			if (!error) {
				cf->running = RUNNING; /*Make a note that program started*/
				execvp(cf->name, cf->args);
			}
			/*If the child's exec did not go well...*/
			perror(cf->name);

			cf->running = STOPPED; /*Note that program ended*/
			error = 1;
			
			exit(1);
		} else {
			processes = processes + 1;
		}
	}

	closeAllPipes(pa);

	if (error)
		killChildren(fs);
	else
		waitOnChildren(fs);

	fflush(stdout);
}

/*changes the parent direcctory to given dname*/
void changeDirectory(input *in) {
	if (in->words[1]) {
		if (chdir(in->words[1]) < 0)
			perror(in->words[1]);
	} else {
		fprintf(stderr, "usage: cd [directory]\n");
	}
}

/********************* Signal Handling *********************/

/*handles sigint*/
/*TODO make waiting a generic function that can take an action*/
void handler(int signum) {
	while (processes){
		/*Wait only updates on child termination*/
		/*Not using waitOnChildren because we no longer care about exit statuses*/
		if(wait(NULL) == -1){
			/*This only occurs if there are no more children running*/
			perror("wait");
			break;
		}
		processes --;
	}
	printf("\n");
	fflush(stdout);
}


/********************* Error Handling *********************/

/*kills all running children*/
void killChildren(fileSet *fs) {
	int i;
	cmdFile *cf;
	for(i=0; i < fs->size;i++) {
		cf = fs->files + i;
		if(cf->running)
			kill(cf->pid, SIGKILL);
	}
}


/********************* Child Handling *********************/

/*The following is only needed so we do not try to kill children that
never started running or were aborted.*/

/*sets a childs running status to STOPPED*/
int updateRunningStatus(int pid, fileSet *fs) {
	int i;
	cmdFile *cf;
	for(i=0; i < fs->size;i++) {
		cf = fs->files + i;
		if(pid == cf->pid){
			cf->running=STOPPED;
			return 0;
		}	
	}
	return 1; /*Could not find the child*/
}

/*waits on the children, kills other children of one exits abnormally*/
/*NOTE: processes can and will be changed after this*/
void waitOnChildren(fileSet *fs){
	int childStatus, exitedPid;
	while (processes){
		if((exitedPid = wait(&childStatus)) != -1 && WIFEXITED(childStatus)){
			if(WEXITSTATUS(childStatus) == 0) { 
				/*If child exited normally*/
				updateRunningStatus(exitedPid, fs);
				processes --;
			} 
			else{
				/*If there was an error, kill all the running processes*/
				killChildren(fs);
				processes = 0;
			}
		} 
		else {
			/* If the child exited by interrupted, signal, etc */
			killChildren(fs);
			processes = 0;
		}
	}
}