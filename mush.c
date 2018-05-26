#include "mush.h"

int activeProcesses = 0;

int main(int argc, char *argv[]) {
	char str[MAX_CMD_LEN];
	fileSet *fs = NULL;
	input *in = NULL;
	FILE *infile = NULL;
	int pipes[MAX_CMD_PIPES][2];

	/*check args*/
	if (argc == 1)
		infile = stdin;
	else if (argc == 2) {
		if (!(infile = fopen(argv[1], "r"))) {
			perror("invalid in file\n");
			return 1;
		}
	} else {
		perror("usage: mush [file]\n");
		return 1;
	}


	/*TODO: loop until EOF, and do more than just piping*/
	while (1) {
		str[MAX_CMD_LEN - 1] = '\0';

		printf("line: ");
		
		if (!(fgets(str, MAX_CMD_LEN, stdin))) {
			perror("no input given\n");
			return 1;
		}

		if (str[MAX_CMD_LEN - 1] != '\0') {
			perror("command too long\n");
			return 1;
		}

		fs = parseInput(in, str);

		printPipeline(fs);
	}




	free(fs);
	free(in);

	return 0;
}