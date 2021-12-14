#ifndef _STR_H_
#define _STR_H_

#include <stdlib.h>

#include "sish.h"

void
delimit_by_pipe( struct command_struct*);

void
delimit_by_redirect(struct command_struct*);

void
delimit_by_space(struct command_struct*);

char*
trim_str(char*);


#endif /* !_STR_H_  */
