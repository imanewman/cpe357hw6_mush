#include "mush.h"

int main(int argc, char *argv[]) {
	char str[MAX_CMD_LEN];
	fileSet *fs = NULL;
	input *in = NULL;
	FILE *infile = NULL;
	pipeArr pa;
	int repeat = 1;
	struct sigaction sa;

	/*check args*/
	if (argc == 1)
		infile = stdin;
	else if (argc == 2) {
		if (!(infile = fopen(argv[1], "r"))) {
			perror(argv[1]);
			return 1;
		}
	} else {
		fprintf(stderr, "usage: mush [file]\n");
		return 1;
	}

	initPipeArr(&pa);

	/*set up sigint signal handling*/
	memset(&sa, 0, sizeof(sa));
	sigfillset(&sa.sa_mask);
	sigdelset(&sa.sa_mask, SIGINT);
	sa.sa_flags = 0;
	sa.sa_handler = &handler;
	sigaction(SIGINT, &sa, &old_sa);

	do {
		str[MAX_CMD_LEN - 1] = '\0';

		/*Only act as a console when directly typing*/
		if(isatty(STDIN_FILENO) && isatty(STDOUT_FILENO))
			printf("8-P ");
		errno = 0;
		if (!(fgets(str, MAX_CMD_LEN, stdin))) { /*check if EOF reached*/
			if(errno != EINTR)
				repeat = 0;
		} else {
			if (str[MAX_CMD_LEN - 1] != '\0') { /*check if cmd was to long*/
				fprintf(stderr, "command too long\n");
				return 1;
			}

			if(strlen(str) > 1)
				in = initInput(str);
			else
				continue;

			if (strcmp("cd", in->words[0])) { /*if command is not 'cd'*/
				if ((fs = parseInput(in))) {
					execProcesses(fs, &pa);

					clearFileSet(fs);
				}
			} else  /*else, cd to given dir*/
				changeDirectory(in);

			clearInput(in);
			processes = 0;
		}
	} while (repeat);

	free(fs);
	free(in);
	fclose(infile);

	return 0;
}