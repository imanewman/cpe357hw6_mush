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