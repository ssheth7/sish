
#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sish.h"

void
parse_input(char* input, struct command_struct* command, size_t buflen)
{
	regex_t preg;
	char *pattern = "[ ]+";
	char *inputcommand;
	char out[256];
	int rc, start, end, index, tokenlen;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	
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
		strncpy(out, inputcommand, start);  // copy the substring to print
       		out[start] = '\0';       // terminate the string
		if ((command->tokenized[index] = malloc(strlen(out) + 1)) == NULL) {
			exit(EXIT_FAILURE);
		}
		tokenlen = strlen(out) + 1;
		strncpy(command->tokenized[index], out, tokenlen);
		printf("tokenized[%d]: <%s>\n", index, command->tokenized[index]);
		inputcommand += end;      // seek the pointer to the start of the next token
		index++;
	}
    	// print the last remaining portion
    	if (strlen(inputcommand) > 0) {
		tokenlen = strlen(inputcommand) + 1;
		if ((command->tokenized[index] = malloc(tokenlen)) == NULL) {
			exit(EXIT_FAILURE);
		}
		strncpy(command->tokenized[index], inputcommand, tokenlen);
        	printf("tokenized[%d]: <%s>\n", index, command->tokenized[index]);
   	}
	command->tokenized[index + 1] = NULL;
	command->num_tokens = index + 2;
   	regfree(&preg);
	free(inputcommand);
}

char*
trim_str(char* input, size_t buflen)
{
	//cint start, end, i, j;
	// int startfound, endfound;
	(void)buflen;
	/*
	end = strlen(input);
	startfound = endfound = start = 0;
	
	for (i = 0, j = end - 1; i < strlen(input); i++, j--) {
		if (input[i] == " ") {
			start++;	
		} else {
			startfound = 1;
		}

		if (input[j] == " ") {
			end = j - 1;
		} else {
			endfound = 1;
		}
	}*/
	return input; 
}
