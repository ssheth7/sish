#ifndef _STR_H_
#define _STR_H_

#include <stdlib.h>

#include "sish.h"

int
delimit_by_pipe( struct command_struct*);

int
delimit_by_redirect(struct command_struct*);

int
delimit_by_space(struct command_struct*);

char*
trim_str(char*);


#endif /* !_STR_H_  */
