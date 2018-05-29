#include "mush.h"

/********************* Pipes *********************/

/*initializes a pipeArr object*/
void initPipeArr(pipeArr *pa) {
	pa->pipes[0] = pa->pipe_0;
	pa->pipes[1] = pa->pipe_1;
	pa->pipes[2] = pa->pipe_2;
	pa->pipes[3] = pa->pipe_3;
	pa->pipes[4] = pa->pipe_4;
	pa->pipes[5] = pa->pipe_5;
	pa->pipes[6] = pa->pipe_6;
	pa->pipes[7] = pa->pipe_7;
	pa->pipes[8] = pa->pipe_8;
}

/*opens all possible pipes*/
void openPipes(pipeArr *pa) {
	int i;

	for (i = 0; i < MAX_CMD_PIPES - 1; i++) {
		if (pipe(pa->pipes[i]) < 0)
			fprintf(stderr, "pipe %d creation error\n", i);
	}
}

/*closes unused pipes based on stage
  if end is 1, closes pipes used by that stage*/
void closePipes(pipeArr *pa, int stage, int end) {
	int i, max = MAX_CMD_PIPES - 2;

	if ((!end && stage != 0) || (end && stage == 0))
		close(pa->pipes[0][WR_END]);

	for (i = 1; i < max; i++) {
		if ((!end && stage != i) || (end && stage == i))  {
			close(pa->pipes[i - 1][RD_END]);
			close(pa->pipes[i][WR_END]);
		}
	}

	if ((!end && stage != max) || (end && stage == max))
		close(pa->pipes[max - 1][RD_END]);
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
			closePipes(pa, i, 0); /*close unneeded pipes*/

			/*set up file input*/
			if (cf->inStage != -1)
				dup2(pa->pipes[i - 1][RD_END], STDIN_FILENO);
			else if (cf->inName) {
				if ((fdin = open(cf->inName, O_RDONLY)) < 0) {
					fprintf(stderr, "%s: cant open \n", cf->inName);

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
					fprintf(stderr, "%s: cant open \n", cf->outName);

					error = 1;
				} else {
					dup2(fdout, STDOUT_FILENO);

					close(fdout);
				}
			}

			closePipes(pa, i, 1);

			if (!(error)) /*exec if no errors thus far*/
				execvp(cf->name, cf->args);

			error = 1;
			fprintf(stderr, "%s: cant exec \n", cf->name);
			exit(1);
		} else { /*in parent*/
			processes++;
		}
	}

	if (error) {
		/*TODO: kill each aready running process if necessary*/
	} else {
		for (i = 0; i < processes; i++)
			waitpid(-1, NULL, 0);
	}

	closePipes(pa, 0, 0);
	closePipes(pa, 0, 1);
}

/*changes the parent direcctory to given dname*/
void changeDirectory(input *in) {
	if (in->words[1]) {
		if (chdir(in->words[1]) < 0)
			fprintf(stderr, "%s: cant change to directory\n", in->words[1]);
	} else {
		perror("missing directory name\n");
	}
}

/********************* Signal Handling *********************/

/*handles sigint*/
void handler(int signum) {
	int i;

	for (i = 0; i < processes; i++)
		waitpid(-1, NULL, 0);
}