#include "mush.h"

/********************* Input *********************/

/*initializes the input string, setting spaces to null*/
/*adds pointers to the word array for each word*/
void initInput(char *str, input *in) {
	in->size = 0;

	/*set a pointer to each word, while setting all spaces and \n to null*/
	in->words[in->size++] = strtok(str, " \n");

	while ((in->words[in->size++] = strtok(NULL, " \n"))) {
		if (in->size == (MAX_CMD_PIPES * (MAX_CMD_ARGS + 3) + 1) 
			&& in->words[in->size - 1] != NULL) {
			/*perror("pipeline too deep or too many arguments\n");*/
			break;
		}
	}

	in->size--; /*decrement for last null pointer*/
}

/*checks for any errors in the input string*/
/*returns 0 if none, 1 if errors*/
int inputErrorCheck(input *in) {
	int pipelen = 0;
	int cmdargs = 0;
	int inset = 0; /*set to 1 for pipe, 2 for redirect*/
	int outset = 0; /*set to 1 for pipe, 2 for redirect*/
	char *curCmd;
	int i;

	for (i = 0; i < in->size; i++) {
		if (cmdargs == 0) { /*if new cmd*/
			/*check if first arg is missing*/
			if (in->words[i][0] == '|' || in->words[i][0] == '<' || in->words[i][0] == '>') {
				fprintf(stderr, "Invalid null command.\n"); 
				return 1;
			} else {
				curCmd = in->words[i];
				cmdargs++;

				if (++pipelen > MAX_CMD_PIPES) { /*check if pipe too long*/
					fprintf(stderr, "Pipeline too deep.\n");
					return 1;
				}
			}
		} else { /*if same cmd*/
			switch (in->words[i][0]) {
				case '|': 
					if (outset == 2) { /*if out already set*/
						fprintf(stderr, "%s: Ambiguous output.\n", curCmd);
						return 1;
					} else { /*reset cmdargs if new cmd coming*/
						cmdargs = 0;
						inset = 1;
						outset = 0;
					}
					break;
				case '<':
					if (inset == 1) { /*if in already set by pipe*/
						fprintf(stderr, "%s: Ambiguous input\n", curCmd);
						return 1;
					} else if (inset == 2) { /*if in already set by redir*/
						fprintf(stderr, "%s: Bad input redirection.\n", curCmd);
						return 1;
					} else { /*if not already set*/
						inset = 2;
						if (in->words[i+1][0] == '|' || in->words[i+1][0] == '<' /* if name is missing*/
							|| in->words[i+1][0] == '>' || in->words[i+1][0] == '\0') {
							fprintf(stderr, "%s: Bad input redirection.\n", curCmd);
							return 1;
						}
					}
					break;
				case '>': 
					if (outset == 2) { /*if out already set by redir*/
						fprintf(stderr, "%s: Bad input redirection.\n", curCmd);
						return 1;
					} else { /*if not already set*/
						outset = 2;
						if (in->words[i+1][0] == '|' || in->words[i+1][0] == '<' /* if name is missing*/
							|| in->words[i+1][0] == '>' || in->words[i+1][0] == '\0') {
							fprintf(stderr, "%s: Bad input redirection.\n", curCmd);
							return 1;
						}
					}
					break;
				default:
					if (++cmdargs > MAX_CMD_ARGS) { /*check if args too long*/
						fprintf(stderr, "%s: Too many arguments.\n", curCmd);
						return 1;
					}
			}

		}
	}

	return 0;
}

/*clears the input struct data*/
void clearInput(input *in) {
	memset(in, 0, sizeof(input));
}

/********************* FileSet *********************/

/*makes the fileset based on the in string*/
void makeFileSet(input *in, fileSet *fs) {
	int i;
	int curCmd = 0;
	cmdFile *cf = NULL;

	for (i = 0; i < in->size; i++) {
		switch (in->words[i][0]) {
			case '|':
				cf->outStage = curCmd + 1;
				fs->files[curCmd + 1].inStage = curCmd;

				cf = NULL;
				curCmd++;
				break;
			case '>':
				cf->outName = in->words[++i];
				break;
			case '<':
				cf->inName = in->words[++i];
				break;
			default:
				if (!(cf)) { /*if file isnt initialized*/
					cf = fs->files + curCmd;
					cf->name = in->words[i];
					cf->stage = curCmd;
					fs->size++;
				}
				
				cf->args[cf->argc++] = in->words[i];
		}
	}
}

/*clears the fileset data*/
void clearFileSet(fileSet *fs) {
	int i;
	fs->size = 0;
	for (i = 0; i < MAX_CMD_PIPES; i++) {
		initCmdFile(fs->files + i);
	}
}

/*checks for errors in input, and returns fileset pointer
  returns null if errors*/
int parseInput(input *in, fileSet *fs) {
	if ((inputErrorCheck(in)))
		return 1;

	makeFileSet(in, fs);
	return 0;
}

/********************* CmdFile *********************/

/*inits a cmdFile to base values, where stage numbers are 
equal to -1 and everything else is empty*/
void initCmdFile(cmdFile *cf) {
	memset(cf, 0, sizeof(cmdFile));

	cf->stage = -1;
	cf->inStage = -1;
	cf->outStage = -1;
}