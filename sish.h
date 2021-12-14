#ifndef _SISH_H_
#define _SISH_H_

#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define usage "sish [-x] [-c command]\n"
#define MAX_TOKENS    256
#define MAX_TOKENLEN  128
#define MAX_COMMANDS  32

#define CD_BUILTIN   "cd"
#define ECHO_BUILTIN "echo"
#define EXIT_BUILTIN "exit"

int EXIT_STATUS;

struct passwd *pwd;

typedef struct sish_flags {
	char* c; /* command  */
	int x;   /* tracing  */
} sish_flags;

sish_flags flags;
typedef struct command_struct {
	char* raw;
	char** tokenized;
	int exit_code;
	int num_pipes;
	int num_tokens;
	int pipe_indexes[32];
} command_struct;


int main(int, char**);




#endif /* !_SISH_H_  */
