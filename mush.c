#include "mush.h"

int activeProcesses = 0;

int main(int argc, char *argv[]) {
	char str[MAX_CMD_LEN];
	fileSet *fs = NULL;
	input *in = NULL;
	FILE *infile = NULL;
	int pipes[MAX_CMD_PIPES][2];
	int repeat = 1;

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

	/*TODO: set up sigint handler*/

	do {
		str[MAX_CMD_LEN - 1] = '\0';

		printf("8-P ");
		
		if (!(fgets(str, MAX_CMD_LEN, stdin))) { /*check if EOF reached*/
			repeat = 0;
		} else {
			if (str[MAX_CMD_LEN - 1] != '\0') { /*check if cmd was to long*/
				perror("command too long\n");
				return 1;
			}

			in = initInput(str);

			if (strcmp("cd", in->words[0])) { /*make pipes if not cd*/
				fs = parseInput(in);

				execProcesses(fs, pipes);
			} else { /*else cd to given dir*/
				if (in->words[1])
					changeDirectory(in->words[1]);
				else 
					perror("missing file name\n");
			}

			clearInput(in);
			clearFileSet(fs);
			activeProcesses = 0;
		}
	} while (repeat);

	free(fs);
	free(in);
	fclose(infile);

	return 0;
}