
#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sish.h"


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

void
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

	strncpy(inputcommand, command->raw, commandlen + 1);
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
		strncpy(tokenized[index], inputcommand, start);
		tokenized[index][tokenlen] = '\0';
		inputcommand += end;      // seek the pointer to the start of the next token
		index++;
		
		tokenized[index] = "|";
		command->pipe_indexes[command->num_pipes] = index;
		index++;
		command->num_pipes++;
		
	}
    	// print the last remaining portion
    	if (strlen(inputcommand) > 0) {
		tokenlen = strlen(inputcommand);
		if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");;
		}
		strncpy(tokenized[index], inputcommand, tokenlen);
		tokenized[index][tokenlen] = '\0';
   		index++;
	}
	tokenized[index + 1] = NULL;
	command->tokenized = tokenized;
	command->num_tokens = index + 1;
   	regfree(&preg);
	free(inputcommand);
}

void
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
		printf("index : %d current pgroup %s\n", index, current_pgroup);
		if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
			err(EXIT_FAILURE, "regcomp");
		}
		while (!(rc = regexec(&preg, current_pgroup, nmatch, pmatch, 0))) {
       		 	start = pmatch[0].rm_so;
			end   = pmatch[0].rm_eo;
			tokenlen = start;
			if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
				exit(EXIT_FAILURE);
			}
			strncpy(tokenized[index], current_pgroup, start);
			tokenized[index][start] = '\0';
			if (end - start == 2) {
				if (current_pgroup[start] == '>' && current_pgroup[end - 1] == '>') {
					tokenized[index + 1] = ">>";
				} else {
					fprintf(stderr, "syntax error near %c\n", current_pgroup[start]);
					return;
				}
			} else if (end - start == 1){
				if (current_pgroup[start] == '>') {
					tokenized[index + 1] = ">";
				} else {
					tokenized[index + 1] = "<";
				}
			} else {
				fprintf(stderr, "syntax error near %c\n", current_pgroup[start]);
				return;
			}
			current_pgroup += end;      // seek the pointer to the start of the next token
			index+=2;
			command->num_pipes++;
			
		}
    		// prin	t the last remaining portion
    		if (strlen(current_pgroup) > 0) {
			tokenlen = strlen(current_pgroup);
		if ((tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");
		}
		strncpy(tokenized[index], current_pgroup, tokenlen);
		tokenized[index][tokenlen] = '\0';
		index++;
   //     	printf("tokenized[%d]: `%s` starts at %d\n", index, tokenized[index], start);
   		}
		tokenized_index++;
	}
	tokenized[index + 1] = NULL;
	command->tokenized = tokenized;
	command->num_tokens = index + 1;
   	regfree(&preg);
}
