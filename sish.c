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
#include "builtins.h"
#include "sish.h"
#include "str.h"

command_struct*
create_command_struct(char* input)
{
	int inputlen;
	struct command_struct *command;
	inputlen = strlen(input) + 1;
	if ((command = calloc(1, sizeof(command))) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	if ((command->raw = calloc(1, sizeof(char*) * (inputlen))) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	if ((command->tokenized = calloc(MAX_TOKENS, MAX_TOKENLEN)) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	strncpy(command->raw, input, inputlen);
	command->raw[inputlen] = '\0';
	return command;
}

int
execute_command(struct command_struct* command) 
{
	char* initcommand;
	int status;
	pid_t pid;
	
	initcommand = command->tokenized[0];
	if (strncmp(initcommand, CD_BUILTIN, strlen(initcommand)) == 0) {
		return cd(command);
	}
	/*for (int i = 0; i < command->num_tokens; i++) {
		printf("tokenized[%d]: %s\n", i, command->tokenized[i]);
	}*/
	if ((pid = fork()) == -1) {
		err(EXIT_FAILURE, "fork");	
	}
	if (pid == 0) {
		execvp(command->tokenized[0], command->tokenized);
		if (errno == ENOENT) {
			fprintf(stderr, "%s: %s: command not found\n", getprogname(), command->raw);
		} else {
			fprintf(stderr, "%s: %s: %s\n", getprogname(), command->raw, strerror(errno));
		}
		(void)exit(127);
	}
	
	if ((pid = waitpid(pid, &status, 0)) < 0) {
		err(EXIT_FAILURE, "waitpid");
	} 
	command->exit_code = status;
	//printf("%s exited with %d\n", command->raw, status);
	return status;
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
	EXIT_STATUS = execute_command(command);
	if (EXIT_STATUS == 127) {
		fprintf(stderr, "%s: %s: command not found\n", getprogname(), command->tokenized[0]);
	}
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
	
	pwd = getpwuid(getuid());
		
	if (flags.c) {
		struct command_struct *command  = create_command_struct(flags.c);
		parse_input(command->raw, command,  strlen(command->raw) + 1);
		execute_command(command);
		exit(EXIT_SUCCESS);
	}
	
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
