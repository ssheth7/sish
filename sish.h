#ifndef _SISH_H_
#define _SISH_H_

#include <sys/types.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define usage "sish [-x] [-c command]\n"

int EXIT_STATUS;

typedef struct sish_flags {
	char* c; /* command  */
	int x;   /* tracing  */
} sish_flags;

sish_flags flags;

int main(int, char**);




#endif /* !_SISH_H_  */
