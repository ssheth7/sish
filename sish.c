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
	if ((command = calloc(1, ARG_MAX)) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	if ((command->raw = calloc(1, sizeof(char*) * (inputlen))) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	strncpy(command->raw, input, inputlen);
	command->raw[inputlen] = '\0';
	command->num_pipes = 0;
	return command;
}

int
execute_command(struct command_struct* command) 
{
	char* token;
	int end_index, start_index, status, i, j;
	int openflags;
	pid_t pid;
	sigset_t nmask, omask;
	
	
	if (sigemptyset(&nmask) < 0) {
		err(EXIT_FAILURE, "sigemptyset");
	}
	if (sigaddset(&nmask, SIGCHLD) < 0) {
		err(EXIT_FAILURE, "sigaddset");
	}	
	if (sigprocmask(SIG_BLOCK, &nmask, &omask) < 0) {
		err(EXIT_FAILURE, "sigprocmask");
	}

	char *subcommand[ARG_MAX];
	int subcommandindex;
	char* redirectfile = NULL;
	char* stdinfile = NULL;
	/* For each command  */	
	openflags = O_CREAT | O_RDWR;
	for (i = 0; i < command->num_pipes - 1; i++) {
		start_index = command->pipe_indexes[i];
		end_index = command->pipe_indexes[i + 1];
		if (end_index == -1) {
			end_index = command->num_tokens;
		}
		if (start_index != 0) {
			start_index ++;
		}
		//printf("group %d pipe_index: %d\n", i, start_index);
		subcommandindex = 0;
		/* Process each from left to right */
		for (j = start_index; j < end_index; j++) {
			token = command->tokenized[j];
			//printf("\tgroup: %d token_index: %d token: %s\n", i, j, token);
			if (subcommandindex > 0 && token) {
				if (strncmp(subcommand[0], ECHO_BUILTIN, strlen(subcommand[0]) != 0) && token[i] == ' ') {
					continue;
				}
			}
			if (token && strncmp(token, "<", strlen(token) ) == 0) {
				stdinfile = command->tokenized[j+1];
				printf("stdin from %s\n", command->tokenized[j+1]);
				j++;
			} else if (token && strncmp(token,  ">", strlen(token)) == 0) {
				redirectfile = command->tokenized[j+1];
				printf("redirect into %s\n", command->tokenized[j+1]);
				j++;
			} else if (token && strncmp(token, ">>", strlen(token)) == 0) {
				redirectfile = command->tokenized[j+1];	
				printf("append to %s\n", command->tokenized[j+1]);
				openflags |= O_APPEND;
				j++;
			} else if (token) {
				subcommand[subcommandindex] = token;
				//printf("subcommand: %s\n", subcommand[subcommandindex]);
				subcommandindex++;
			} else  {
				continue;
			}
		}
		subcommand[subcommandindex] = NULL;
		if ((pid = fork()) == -1) {
			err(EXIT_FAILURE, "fork");	
			/* NOT REACHED  */
		}
		/* child process  */
		if (pid == 0) { 
			int outputfd;	
			int inputfd;
			if (redirectfile) {
				if ((outputfd = open(redirectfile, openflags, S_IRUSR | S_IWUSR))  == -1) {
					fprintf(stderr, "%s: %s: No such file or directory\n",getprogname(), redirectfile);
					return EXIT_FAILURE; 
				}
				if (dup2(outputfd, STDOUT_FILENO) != STDOUT_FILENO) {
					err(EXIT_FAILURE, "dup2 to stdout");
				}
				close(outputfd);
			}	
			if (stdinfile) {
				if ((inputfd = open(stdinfile, O_RDONLY)) == -1) {
					fprintf(stderr, "%s: %s: No such file or directory\n", getprogname(), stdinfile);
					return EXIT_FAILURE;
				}
				if (dup2(inputfd, STDIN_FILENO) != STDIN_FILENO) {
					err(EXIT_FAILURE, "dup2 to stdin");
				}
				close(inputfd);	
			}
			if (strncmp(subcommand[0], CD_BUILTIN, strlen(subcommand[0])) == 0) {
				return cd(subcommand, subcommandindex);
			} else if (strncmp(subcommand[0], ECHO_BUILTIN, strlen(subcommand[0])) == 0) {
				return echo(subcommand, subcommandindex);
			}
			execvp(subcommand[0], subcommand);
			return 127;
		}
		/* parent process  */
		if (pid > 0) {
			if (waitpid(pid, &status, 0) < 0) {
				err(EXIT_FAILURE, "waitpid");
			}
		}
	}
	command->exit_code = status;
	return status;
}

int
process_input(char *buffer) 
{
	int i, pipe_index;
	struct command_struct *command;
			
	command  = create_command_struct(buffer);
	delimit_by_pipe(command);
	delimit_by_redirect(command);
	delimit_by_space(command);
	command->pipe_indexes[0] = 0;
	for (i = 0, pipe_index = 1; i < command->num_tokens - 1; i++) {
		//printf("%d: %s\n", i, command->tokenized[i]);
		if (strncmp(command->tokenized[i], "|", strlen(command->tokenized[i])) == 0) {
			//printf("pipe #%d at token %d\n", pipe_index, i);
			command->pipe_indexes[pipe_index++] = i;
		}
	}
	command->pipe_indexes[pipe_index] = -1;
	command->num_pipes = pipe_index + 1;
	//printf("num_pipes: %d\n", pipe_index + 1);
	EXIT_STATUS = execute_command(command);
	if (EXIT_STATUS == 127) {
		fprintf(stderr, "%s: %s: command not found\n", getprogname(), command->tokenized[0]);
	}
	for (i = 0; i < command->num_tokens - 1; i++) {
		free(command->tokenized[i]);
	}
	return 0;
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
		if (strlen(flags.c) == 0) {
			return 0;
		}
		if (strncmp(flags.c, EXIT_BUILTIN, strlen(flags.c)) == 0) {
			exit_sish();
		}
		process_input(flags.c);	
		return EXIT_STATUS;
	}
	
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "signal");
	}

	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		err(EXIT_FAILURE, "signal");
	}
	
	char *input;
	for (;;) {
		
		printf("sish$ ");
		if ((input = fgets(buf, ARG_MAX, stdin)) == NULL) {
			if (feof(stdin) == 0) { 
				err(EXIT_FAILURE, "fgets"); 
			} else {
				printf("\n");
				exit_sish();	
			}
		}
		if (strncmp(buf, "\n", strlen(buf)) == 0) {
			return 0;
		}
		buf[strlen(buf) - 1] = '\0';
		if (strncmp(input, EXIT_BUILTIN, strlen(input)) == 0) {
			exit_sish();
		}
		process_input(buf);
	}
	return EXIT_STATUS;
}
