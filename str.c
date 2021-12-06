#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char*
parse_input(char* input, size_t buflen)
{
	regex_t preg;
	char *pattern = "[ ]+";
	char *inputcommand;
	char out[256];
	int rc;
	size_t nmatch = 1;
	regmatch_t pmatch[1];
	(void)buflen;
	
	if ((inputcommand = malloc(buflen)) == NULL) {
		err(EXIT_FAILURE, "malloc");
	}
	strncpy(inputcommand, input, buflen);
	inputcommand[strlen(input)] = '\0';
	if ((rc = regcomp(&preg, pattern, REG_EXTENDED)) != 0) {
		err(EXIT_FAILURE, "regcomp");
	}


	while (!(rc = regexec(&preg, inputcommand, nmatch, pmatch, 0))) {
        	strncpy(out, input, pmatch[0].rm_so);  // copy the substring to print
       		out[pmatch[0].rm_so] = '\0';       // terminate the string
        	//printf("\"%s\" matches  characters %ld to %ld\n",inputcommand, pmatch[0].rm_so, pmatch[0].rm_eo);
        	printf("out: %s\n", out);
		inputcommand += pmatch[0].rm_eo;      // seek the pointer to the start of the next token
   	}
    	// print the last remaining portion
    	/*if (strlen(input) > 0) {
        	printf("%s\n", input);
   	}*/
   	regfree(&preg);
	return input;
}

char*
trim_str(char* input, size_t buflen)
{

}
