/*
 * Includes string parsing and alterating methods
*/
#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "sish.h"

/*
 * Returns a pointer to trimmed input
*/
char*
trim_str(char* input)
{
	int start, i, end, newend;
	
	end = strlen(input);
	
	start = newend = 0;
	for (i = 0; i < end; i++) {
		if (input[i] != ' ' && input[i] != '\t') {
			break;
		}
		start++;
	}
	for (i = end - 1; i >= 0; i--) {
		if (input[i] != ' ' && input[i] != '\t') {
			break;
		}
		newend = i;
	}
	if (newend > i) {
		input[newend] = '\0';
	}
	return input + start; 
}

/*
 * Uses regex to delimit a string into a tokenized array
 *  "ls -la | wc -l" -> {"ls -la", "|", "wc -l"}
*/
int
delimit_by_pipe(struct command_struct* command)
{
	regex_t preg;
	char *pattern = "[|]+";
	char *inputcommand;
	char **tokenized;
	int rc, start, end, index, tokenlen;
	int commandlen;
	size_t nmatch = 1;
	regmatch_t pmatch[1];

	command->raw = trim_str(command->raw);	
	commandlen = strlen(command->raw);	
	if ((inputcommand = malloc(commandlen + 1)) == NULL) {
		err(EXIT_FAILURE, "malloc");
	}

	(void)strncpy(inputcommand, command->raw, commandlen + 1);
	inputcommand[strlen(command->raw)] = '\0';

	if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
		err(EXIT_FAILURE, "regcomp");
	}
	if ((tokenized = calloc(MAX_TOKENS, MAX_TOKENLEN)) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}

	index = 0;
	while (!(rc = regexec(&preg, inputcommand, nmatch, pmatch, 0))) {
        	start = pmatch[0].rm_so;
		end   = pmatch[0].rm_eo;
		tokenlen = start;
		if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			exit(EXIT_FAILURE);
		}
		(void)strncpy(tokenized[index], inputcommand, start);
		tokenized[index][tokenlen] = '\0';
		index++;
		/* Add following pipe into tokenized */
		if ((tokenized[index] = malloc(sizeof(char*) * 2)) == NULL) {
			exit(EXIT_FAILURE);
		}	
		strncpy(tokenized[index], "|", 2); 
		tokenized[index][2] = '\0';
		index++;
		inputcommand += end;     		
	}
    	
	/* Add trailing characters into tokenized */
	if (strlen(inputcommand) > 0) {
		tokenlen = strlen(inputcommand);
		if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");
		}
		(void)strncpy(tokenized[index], inputcommand, tokenlen);
		tokenized[index][tokenlen] = '\0';
   		index++;
	}
	tokenized[index + 1] = NULL;
	command->tokenized = tokenized;
	command->num_tokens = index + 1;
   	regfree(&preg);
	return EXIT_SUCCESS;
}

/*
 * Delimits a string array by redirects
 *  {"< file cat", "|", "wc -l" } -> {"<", "file cat", "|", "wc -l"} 
*/
int
delimit_by_redirect(struct command_struct* command)
{

	regex_t preg;
	char *pattern = "[<>]+";
	char *current_pgroup;
	char **tokenized;
	int rc, start, end, index, tokenlen;
	int tokenized_index;
	size_t nmatch = 1;
	regmatch_t pmatch[1];

	if ((tokenized = calloc(MAX_TOKENS, MAX_TOKENLEN)) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	index = 0;
	tokenized_index = 0;

	while ((current_pgroup = command->tokenized[tokenized_index])) {
		if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
			err(EXIT_FAILURE, "regcomp");
		}
		while (!(rc = regexec(&preg, current_pgroup, nmatch, pmatch, 0))) {
       		 	start = pmatch[0].rm_so;
			end   = pmatch[0].rm_eo;
			tokenlen = start;
			
			/* Add characters in between delimiter  */
			if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
				exit(EXIT_FAILURE);
			}
			(void)strncpy(tokenized[index], current_pgroup, start);
			tokenized[index][start] = '\0';
			if (start == 0) {
				index--;
			}	
			/* Add delimiter to tokenized  */
			if (end - start == 2) {
				if (current_pgroup[start] == '>' && current_pgroup[end - 1] == '>') {
					tokenized[index + 1] = ">>";
				} else {
					fprintf(stderr, "syntax error near %c\n", current_pgroup[start]);
					return EXIT_FAILURE;
				}
			} else if (end - start == 1){
				if (current_pgroup[start] == '>') {
					tokenized[index + 1] = ">";
				} else {
					tokenized[index + 1] = "<";
				}
			} else {
				fprintf(stderr, "syntax error near %c\n", current_pgroup[start]);
				return EXIT_FAILURE;
			}
			current_pgroup += end;      // seek the pointer to the start of the next token
			index+=2;
		}
    		// Print trailing characters
    		if (strlen(current_pgroup) > 0) {
			tokenlen = strlen(current_pgroup);
		if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");
		}
		(void)strncpy(tokenized[index], current_pgroup, tokenlen);
		tokenized[index][tokenlen] = '\0';
		index++;
   		}
		tokenized_index++;
	}
	tokenized[index + 1] = NULL;
	command->tokenized = tokenized;
	command->num_tokens = index + 1;
	regfree(&preg);
	return EXIT_SUCCESS;
}

/*
 * Delimits a string array by spaces
 *  {"<", "file cat", "|", "wc -l"}  -> {"<", "file", " ", "cat", "|", "wc", "-l"}
*/
int
delimit_by_space(struct command_struct* command)
{
	regex_t preg;
	char *pattern = "[ ]+";
	char *current_pgroup;
	char **tokenized;
	int rc, start, end, index, tokenlen;
	int tokenized_index, num_spaces;
	size_t nmatch = 1;
	regmatch_t pmatch[1];

	if ((tokenized = calloc(MAX_TOKENS, MAX_TOKENLEN)) == NULL) {
		err(EXIT_FAILURE, "calloc");
	}
	index = 0;
	tokenized_index = 0;
	while ((current_pgroup = command->tokenized[tokenized_index])) {
		if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
			err(EXIT_FAILURE, "regcomp");
		}
		while (!(rc = regexec(&preg, current_pgroup, nmatch, pmatch, 0))) {
       		 	start = pmatch[0].rm_so;
			end   = pmatch[0].rm_eo;
			num_spaces = end - start;
			tokenlen = start;
			
			if ((tokenized[index] = malloc(sizeof(char) * MAX_TOKENLEN)) == NULL) {
				(void)err(EXIT_FAILURE, "malloc");
			}
			strncpy(tokenized[index], current_pgroup, start);
			tokenized[index][start] = '\0';
			if (start == 0) {
				index--;
			}
			index++;
			/* Add spaces to tokenized  */
			if ((tokenized[index] = malloc(sizeof(char) * MAX_TOKENLEN)) == NULL) {
				(void)err(EXIT_FAILURE, "malloc");
			}
			(void)strncpy(tokenized[index], current_pgroup + start, num_spaces);
			tokenized[index][num_spaces + 1] = '\0';
			index++;
			current_pgroup += end;      			
		}
    		/* Add trailing characters to tokenized */
    		if (strlen(current_pgroup) > 0) {
			tokenlen = strlen(current_pgroup);
		if ((tokenized[index] = malloc(sizeof(char) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");
		}
		(void)strncpy(tokenized[index], current_pgroup, tokenlen);
		tokenized[index][tokenlen] = '\0';
		index++;
   		}
		tokenized_index++;
	}
	tokenized[index + 1] = NULL;
	command->tokenized = tokenized;
	command->num_tokens = index + 1;
   	regfree(&preg);
	return EXIT_SUCCESS;
}



