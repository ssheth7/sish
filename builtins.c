/*
 * Contains implementation of built in shell commands: cd, echo, exit
*/
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "sish.h"


/*
 * Changes directory of shell
*/
int 
cd(char **subcommand, int numtokens)
{
	if (numtokens > 3) {
		(void)fprintf(stderr, "%s: cd: too many arguments\n", getprogname());
		return EXIT_FAILURE;
	}
	if (numtokens == 1) {
		if (chdir(pwd->pw_dir) == -1) {
			(void)fprintf(stderr, "%s: cd: %s: No such file or directory\n", getprogname(), pwd->pw_dir);
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}	
	if (chdir(subcommand[1]) == -1) {
		(void)fprintf(stderr, "%s: cd: %s: %s\n", getprogname(), subcommand[1], strerror(errno));
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/*
 * Implementation of built in command echo
*/
int 
echo(char **subcommand, int numtokens)
{
	char* get_last_status = "$?";
	char* get_pid = "$$";
	char* token;
	int i;
	pid_t pid;
	
	pid = getpid();
	for (i = 1; i < numtokens ; i++) {
		token = subcommand[i];
		
		if (strncmp(token, get_last_status, strlen(token)) == 0) {
			(void)printf("%d", EXIT_STATUS);
		} else if (strncmp(token, get_pid, strlen(token)) == 0) {
			(void)printf("%d", pid);
		} else {
			(void)printf("%s", subcommand[i]);	
		}
	}
	printf("\n");
	return EXIT_SUCCESS;
}

/*
 * Exits the shell with last status code
*/
void
exit_sish() {
	(void)exit(EXIT_STATUS);
} 
