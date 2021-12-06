/*
 * World's 2nd simplest shell
 * Same as simple-shell.c, but with a SIGINT signal handler.
 * Feed EOF (^D) to exit.
 *
 * Also illustrates forward declaration of a function prototype.
 */

/*
 * Usage: sish [-x] [-c command]
*/
#include "sish.h"
#include "str.h"


char *
getinput(char *buffer, size_t buflen) 
{
	char *input;

	printf("sish$ ");
	if ((input = fgets(buffer, buflen, stdin)) == NULL) {
		if (feof(stdin) == 0) { 
			err(EXIT_FAILURE, "fgets"); 
		} else {
			buffer[strlen(buffer) - 1] = '\0';
			return input;
		}
	}
	buffer[strlen(buffer) - 1] = '\0';
	parseinput(buffer, buflen);
	return input;
}


int
main(int argc, char **argv) 
{
	char buf[ARG_MAX];
	int opt, status;
	pid_t pid;
	
	(void)setprogname(argv[0]);
	
	while ((opt = getopt(argc, argv,"c:x")) != -1) {
		switch(opt) {
			case 'c': // command
				flags.c = optarg;	
				break;
			case 'x': // tracing
				flags.x = 1;
				break;
			default : 
				(void)printf(usage);
				return EXIT_FAILURE;
				/* NOTREACHED  */
		}
	}
	argc -= optind;
	argc += optind;
	
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "signal");
	}

	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "signal");
	}

	while (getinput(buf, sizeof(buf))) {

		if((pid=fork()) == -1) {
			err(EXIT_FAILURE, "fork");
			continue;
		} else if (pid == 0) {   /* child */
			execlp(buf, buf, (char *)0);
			fprintf(stderr, "shell: couldn't exec %s: %s\n", buf, strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* parent waits */
		if ((pid=waitpid(pid, &status, 0)) < 0) {
			fprintf(stderr, "shell: waitpid error: %s\n", strerror(errno));
		}
	}
	printf("\n");

	exit(EXIT_SUCCESS);
}
