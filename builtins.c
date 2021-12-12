
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "sish.h"


int cd(struct command_struct* command)
{
	if (command->num_tokens > 3) {
		fprintf(stderr, "%s: cd: too many arguments\n", getprogname());
		return EXIT_FAILURE;
	}
	if (command->num_tokens == 2) {
		if (chdir(pwd->pw_dir) == -1) {
			fprintf(stderr, "%s: cd: %s: No such file or directory\n", getprogname(), pwd->pw_dir);
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}	
	if (chdir(command->tokenized[1]) == -1) {
		fprintf(stderr, "%s: cd: %s: %s\n", getprogname(), command->tokenized[1], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int echo(struct command_struct* command)
{
	
	return 0;
}
