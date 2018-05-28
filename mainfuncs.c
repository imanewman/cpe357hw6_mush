#include "mush.h"

/*closes unused pipes basedon stage
  if end is 1, closes pipes used by that stage*/
void closePipes(int **pipes, int stage, int end) {
	int i, max = MAX_CMD_PIPES - 1;

	if ((!end && stage != 0) || (end && stage == 0))
		close(pipes[0][WR_END]);

	for (i = 1; i < max; i++) {
		if ((!end && stage != i) || (end && stage == i))  {
			close(pipes[i][RD_END]);
			close(pipes[i + 1][WR_END]);
		}
	}

	if ((!end && stage != max) || (end && stage == max))
		close(pipes[max][RD_END]);

	if (!end) {
		close(pipes[0][RD_END]);
		close(pipes[max][WR_END]);
	}
}

/*forks and executes the given pipeline of processes*/
void execProcesses(fileSet *fs, int **pipes) {
	int i;
	int error = 0; /*set to 1 if theres a probem and running processes need to be killed*/

	for (i = 0; i < fs->size; i++) {
		/*TODO: exec each process / check for errors*/
	}

	if (error) {
		/*TODO: kill each aready running process if necessary*/
	}
} /*make sure to increment active processes for sigint handler*/

/*changes the parent direcctory to given dname*/
void changeDirectory(input *in) {
	if (in->words[1]) {
		if (chdir(in->words[1]) < 0)
			fprintf(stderr, "%s: cant change to directory\n", in->words[1]);
	} else {
		perror("missing directory name\n");
	}
}

/*handles sigint*/
void handler(int signum) {
	int i;

	for (i = 0; i < processes; i++)
		waitpid(-1, NULL, 0);
}