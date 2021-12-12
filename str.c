
#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sish.h"


char*
trim_str(char* input, size_t buflen)
{
	int start, i, end, newend;;
	// int startfound, endfound;
	(void)buflen;
	
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
parse_input(char* input, struct command_struct* command, size_t buflen)
{
	regex_t preg;
	char *pattern = "[ ]+";
	char *inputcommand;
	int rc, start, end, index, tokenlen;
	size_t nmatch = 1;
	regmatch_t pmatch[1];

	input = trim_str(input, buflen);	
	if ((inputcommand = malloc(buflen)) == NULL) {
		err(EXIT_FAILURE, "malloc");
	}

	strncpy(inputcommand, input, buflen);
	inputcommand[strlen(input)] = '\0';
	if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
		err(EXIT_FAILURE, "regcomp");
	}
	index = 0;
	while (!(rc = regexec(&preg, inputcommand, nmatch, pmatch, 0))) {
        	start = pmatch[0].rm_so;
		end   = pmatch[0].rm_eo;
		tokenlen = start;
		if ((command->tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			exit(EXIT_FAILURE);
		}
		strncpy(command->tokenized[index], inputcommand, start);
		command->tokenized[index][tokenlen] = '\0';
//		printf("tokenized[%d] `%s` starts at %d end %d\n", index, command->tokenized[index],
//		start, end);
		inputcommand += end;      // seek the pointer to the start of the next token
		index++;
	}
    	// print the last remaining portion
    	if (strlen(inputcommand) > 0) {
		tokenlen = strlen(inputcommand);
		if ((command->tokenized[index] = malloc(sizeof(char*) * MAX_TOKENLEN)) == NULL) {
			err(EXIT_FAILURE, "malloc");;
		}
		strncpy(command->tokenized[index], inputcommand, tokenlen);
		command->tokenized[index][tokenlen] = '\0';
   //     	printf("tokenized[%d]: `%s` starts at %d\n", index, command->tokenized[index], start);
   	}
	command->tokenized[index + 1] = NULL;
	command->num_tokens = index + 2;
   	regfree(&preg);
	free(inputcommand);
}
