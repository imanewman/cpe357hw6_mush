#include "mush.h"

/********************* Input *********************/

/*initializes the input string, setting spaces to null*/
/*adds pointers to the word array for each word*/
input *initInput(char *str) {
	input *in = (input *)malloc(sizeof(input));

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

	return in;
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
				perror("invalid null command\n"); 
				return 1;
			} else {
				curCmd = in->words[i];
				cmdargs++;

				if (++pipelen > MAX_CMD_PIPES) { /*check if pipe too long*/
					perror("pipeline too deep\n");
					return 1;
				}
			}
		} else { /*if same cmd*/
			switch (in->words[i][0]) {
				case '|': 
					if (outset == 2) { /*if out already set*/
						fprintf(stderr, "%s: ambiguous output\n", curCmd);
						return 1;
					} else { /*reset cmdargs if new cmd coming*/
						cmdargs = 0;
						inset = 1;
						outset = 0;
					}
					break;
				case '<':
					if (inset == 1) { /*if in already set by pipe*/
						fprintf(stderr, "%s: ambiguous input\n", curCmd);
						return 1;
					} else if (inset == 2) { /*if in already set by redir*/
						fprintf(stderr, "%s: bad input redirection\n", curCmd);
						return 1;
					} else { /*if not already set*/
						inset = 2;
						if (in->words[i+1][0] == '|' || in->words[i+1][0] == '<' /* if name is missing*/
							|| in->words[i+1][0] == '>' || in->words[i+1][0] == '\0') {
							fprintf(stderr, "%s: bad input redirection\n", curCmd);
							return 1;
						}
					}
					break;
				case '>': 
					if (outset == 2) { /*if out already set by redir*/
						fprintf(stderr, "%s: bad output redirection\n", curCmd);
						return 1;
					} else { /*if not already set*/
						outset = 2;
						if (in->words[i+1][0] == '|' || in->words[i+1][0] == '<' /* if name is missing*/
							|| in->words[i+1][0] == '>' || in->words[i+1][0] == '\0') {
							fprintf(stderr, "%s: bad output redirection\n", curCmd);
							return 1;
						}
					}
					break;
				default:
					if (++cmdargs > MAX_CMD_ARGS) { /*check if args too long*/
						fprintf(stderr, "%s: too many arguments\n", curCmd);
						return 1;
					}
			}

		}
	}

	return 0;
}

/*clears the input struct data*/
void clearInput(input *in) {
	int i;

	for (i = 0; i < in->size; i++) 
		in->words[i] = NULL;

	in->size = 0;
}

/********************* FileSet *********************/

/*initializes and returns a pointer to the fileset*/
/*initializes all cmdFiles within*/
fileSet *initFileSet() {
	fileSet *fs = (fileSet *)malloc(sizeof(fileSet));

	clearFileSet(fs);

	return fs;
}

/*makes the fileset based on the in string*/
fileSet *makeFileSet(input *in) {
	fileSet *fs = initFileSet();
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
				cf->startLen += 2;
				break;
			case '<':
				cf->inName = in->words[++i];
				cf->startLen += 2;
				break;
			default:
				if (!(cf)) { /*if file isnt initialized*/
					cf = fs->files + curCmd;
					cf->name = in->words[i];
					cf->stage = curCmd;
					cf->start = in->words + i;
					fs->size++;
				}
				
				cf->args[cf->argc++] = in->words[i];
				cf->startLen++;
		}
	}

	return fs;
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
fileSet *parseInput(input *in) {
	if ((inputErrorCheck(in)))
		return NULL;
		
	return makeFileSet(in);
}

/********************* CmdFile *********************/

/*inits a cmdFile to base values*/
void initCmdFile(cmdFile *cf) {
	int i;

	cf->name = NULL;
	cf->stage = -1;
	cf->start = NULL;
	cf->startLen = 0;
	cf->argc = 0;

	for (i = 0; i < MAX_CMD_ARGS; i++)
		cf->args[i] = NULL;

	cf->inStage = -1;
	cf->outStage = -1;
	cf->inName = NULL;
	cf->outName = NULL;
}

/********************* Printing *********************/

/*prints the stages of the pipeline*/
void printPipeline(fileSet *fs) {
	int i;

	for (i = 0; i < fs->size; i++)
		printStage(fs->files + i);
}

/*prints one stage of the pipeline*/
void printStage(cmdFile *cf) {
	int i;

	printf("\n--------\nStage %d: \"%s", cf->stage, cf->start[0]);

	for (i = 1; i < cf->startLen; i++) 
		printf(" %s", cf->start[i]);

	printf("\"\n--------\n");

	if (cf->inStage != -1)
		printf("input: pipe from stage %d\n", cf->inStage);
	else if (cf->inName)
		printf("input: %s\n", cf->inName);
	else
		printf("input: original stdin\n");

	if (cf->outStage != -1)
		printf("output: pipe to stage %d\n", cf->outStage);
	else if (cf->outName)
		printf("output: %s\n", cf->outName);
	else 
		printf("output: original stdout\n");

	printf("argc: %d\nargv: ", cf->argc);

	if (cf->argc) {
		for (i = 0; i < cf->argc - 1; i++)
			printf("\"%s\",", cf->args[i]);

		printf("\"%s\"\n", cf->args[cf->argc - 1]);
	} else printf("\n");
}