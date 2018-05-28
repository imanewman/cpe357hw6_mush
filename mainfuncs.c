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
}

/*forks and executes the given pipeline of processes*/
void execProcesses(fileSet *fs, int **pipes) {

} /*make sure to increment active processes for sigint handler*/

/*changes the parent direcctory to given dname*/
void changeDirectory(char *dname) {

}

/*handles sigint*/
void handler(int signum) {
	int i;
	
	for (i = 0; i < processes; i++)
		waitpid(-1, NULL, 0);
}