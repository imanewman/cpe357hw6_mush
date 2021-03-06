#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CMD_LEN 512 /*max command line length*/
#define MAX_CMD_PIPES 10 /*max commands in a pipeline*/
#define MAX_CMD_ARGS 10 /*max args to a command*/

#define RD_END 0
#define WR_END 1

/*Used in error handling for starting and stopping children*/
#define RUNNING 1
#define STOPPED 0

int processes;
struct sigaction old_sa;

/********************* Structures *********************/

typedef struct cmdFile {
	char *name;
	short stage;
	pid_t pid;
	char *args[MAX_CMD_ARGS]; /*NULL if not set*/
	short argc;
	short inStage; /*-1 if not set*/
	short outStage; /*-1 if not set*/
	char *inName; /*NULL if not set*/
	char *outName; /*NULL if not set*/
	short running; /*1 if running, 0 otherwise*/
} cmdFile;

typedef struct fileSet {
	cmdFile files[MAX_CMD_PIPES];
	short size;
} fileSet;

typedef struct input {
	char *words[MAX_CMD_PIPES * (MAX_CMD_ARGS + 3) + 1]; /*3 extra args for name, <, >*/
	int size;
} input;

typedef struct pipeArr {
	int pipes[MAX_CMD_PIPES-1][2];
} pipeArr;


/*******************************************************/
/********************* Parse Funcs *********************/
/*******************************************************/

/********************* Input *********************/

/*initializes the given input based on a string, setting 
spaces to null adds pointers to the word array for each word*/
void initInput(char *str, input *in);

/*checks for any errors in the input string
  returns 0 if none, 1 if errors*/
int inputErrorCheck(input *in);

/*clears the input struct data*/
void clearInput(input *in);

/********************* FileSet *********************/

/*makes the fileset based on the in string*/
void makeFileSet(input *in, fileSet *fs);

/*clears the fileset data*/
void clearFileSet(fileSet *fs);

/*checks for errors in input
	returns 0 if none, 1 if errors*/
int parseInput(input *in, fileSet *fs);

/********************* CmdFile *********************/

/*inits a cmdFile to base values*/
void initCmdFile(cmdFile *cf);

/*******************************************************/
/********************* Main Funcs **********************/
/*******************************************************/

/********************* Pipes *********************/

/*initializes a pipeArr object*/
void initPipeArr(pipeArr *pa);

/*opens all possible pipes*/
void openPipes(pipeArr *pa);

/*closes all the pipes*/
void closeAllPipes(pipeArr *pa);

/********************* main functions *********************/

/*forks and executes the given pipeline of processes*/
void execProcesses(fileSet *fs, pipeArr *pa);

/*changes the parent direcctory to given dname*/
void changeDirectory(input *in);

/********************* Signal Handling *********************/

/*handles sigint*/
void handler(int signum);

/********************* Error Handling *********************/

/*kills children*/
void killChildren(fileSet *fs);

/********************* Child Handling *********************/

/*sets the status of a given child to STOPPED*/
int updateRunningStatus(int pid, fileSet *fs);

/*waits on all running children to finish and updates processes*/
void waitOnChildren(fileSet *fs);

