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


int
execute_command(struct command_struct* command) 
{
	int status;
	pid_t pid;
	/*for (int i = 0; i < command->num_tokens; i++) {
		printf("tokenized[%d]: %s\n", i, command->tokenized[i]);
	}*/
	
	if ((pid = fork()) == -1) {
		err(EXIT_FAILURE, "fork");	
	}
	if (pid < 0) {
		execvp(command->tokenized[0], command->tokenized);
		fprintf(stderr, "shell: couldn't exec %s: %s\n", command->raw, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if ((pid = waitpid(pid, &status, 0)) < 0) {
		err(EXIT_FAILURE, "waitpid");
	} 
	//destroy_command(command);
	printf("%s exited with %d\n", command->raw, status);
	return EXIT_SUCCESS;
}

command_struct*
create_command_struct(char* input)
{
	struct command_struct *command;
	if ((command = malloc(sizeof(command))) == NULL) {
		err(EXIT_FAILURE, "malloc");
	}
	command->raw = input;
	if ((command->tokenized = malloc(ARG_MAX + 1)) == NULL) {
		err(EXIT_FAILURE, "malloc");
	}
	return command;
}

void
destroy_command_struct(struct command_struct* command)
{
	int i = 0;
	for (i = 0; i < command->num_tokens - 1; i++) {
		free(command->tokenized[i]);
	}
	free(command->tokenized);
	free(command);
}
int
getinput(char *buffer, size_t buflen) 
{
	char *input;
	struct command_struct *command;
			
	printf("sish$ ");
	if ((input = fgets(buffer, buflen, stdin)) == NULL) {
		if (feof(stdin) == 0) { 
			err(EXIT_FAILURE, "fgets"); 
		} else {
			return 0;
		}
	}
	if (buffer[0] == '\n') {
		return 1;
	}
	buffer[strlen(buffer) - 1] = '\0';
	command  = create_command_struct(buffer);
	parse_input(buffer, command,  buflen);
	execute_command(command);
	//destroy_command_struct(command);
	return 1;
}


int
main(int argc, char **argv) 
{
	char buf[ARG_MAX];
	int opt;
	
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
		;
	}
	printf("\n");

	exit(EXIT_SUCCESS);
}
