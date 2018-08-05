/* public things from cmd.c */

#ifndef _CMD_H_
#define _CMD_H_

typedef struct {
    char *name;			/* command name */
    char *help;			/* short help */
    function_str funct;		/* function to call */
} cmdstruct;

extern cmdstruct commands[];
void init_cmds();

#endif /* _CMD_H_ */

