/*
 *  cmd.c  --  functions for powwow's built-in #commands
 *
 *  (created: Finn Arne Gangstad (Ilie), Dec 25th, 1993)
 * 
 *  Copyright (C) 1998 by Massimiliano Ghilardi
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "powdefines.h"
#include "powhook.h"
#include "powmain.h"
#include "powutils.h"
#include "powbeam.h"
#include "powtty.h"
#include "powcmd.h"
#include "powcmd2.h"
#include "powedit.h"
#include "poweval.h"
#include "powlist.h"
#include "powmap.h"
#include "powlog.h"

/*           local function declarations            */
#define _  (char *arg)

static void cmd_help _, 
  cmd_action _, cmd_add _, cmd_alias _, cmd_at _, cmd_beep _, cmd_bind _,
  cmd_cancel _, cmd_capture _, cmd_cpu _,
  cmd_do _, cmd_delim _, cmd_edit _, cmd_emulate _, cmd_exe _,
  cmd_file _, cmd_for _, cmd_hilite _, cmd_history _, cmd_host _,
  cmd_identify _, cmd_if _, cmd_in _, cmd_init _, cmd_isprompt _,
  cmd_key _, cmd_keyedit _,
  cmd_load _, cmd_map _, cmd_mark _, cmd_movie _,
  cmd_nice _, cmd_option _,
  cmd_print _, cmd_prompt _, cmd_put _,
  cmd_qui _, cmd_quit _, cmd_quote _,
  cmd_rawsend _, cmd_rebind _,cmd_rebindall _,cmd_rebindALL _,
  cmd_record _, cmd_request _, cmd_reset _, cmd_retrace _,
  cmd_save _, cmd_send _, cmd_separator _,cmd_setvar _, cmd_stop _,
  cmd_time _, cmd_var _, cmd_while _, cmd_write _, cmd_eval _;


#undef _

cmdstruct commands[] =
{
    {"17", "command\t\t\trepeat `command' 17 times", (function_str)0},
    {"action", "[[<|=|>|%][+|-]name] [{pattern|(expression)} [=[command]]]\r\n\t\t\t\tdelete/list/define actions", cmd_action},
    {"add", "{string|(expr)}\t\tadd the string to word completion list", cmd_add},
    {"alias", "[name[=[text]]]\t\tdelete/list/define aliases", cmd_alias},
    {"at", "[name [(time-string) [command]]\tset time of delayed label", cmd_at},
    {"beep", "\t\t\t\tmake your terminal beep (like #print (*7))", cmd_beep},
    {"bind", "[edit|name [seq][=[command]]]\tdelete/list/define key bindings", cmd_bind},
    {"cancel", "[number]\t\tcancel editing session", cmd_cancel},
    {"capture", "[filename]\t\tbegin/end of capture to file", cmd_capture},
    {"cpu", "\t\t\t\tshow CPU time used by powwow", cmd_cpu},
    {"delim", "[normal|program|{custom [chars]}]\r\n\t\t\t\tset word completion delimeters", cmd_delim},
    {"do", "(expr) command\t\trepeat `command' (expr) times", cmd_do},
    {"edit", "\t\t\t\tlist editing sessions", cmd_edit},
    {"emulate", "[<|!]{text|(expr)}\tprocess result as if received from host", cmd_emulate},
    {"exe", "[<|!]{text|(string-expr)}\texecute result as if typed from keyboard", cmd_exe},
    {"file", "[=[filename]]\t\tset/show powwow definition file", cmd_file},
    {"for", "([init];check;[loop]) command\twhile `check' is true exec `command'", cmd_for},
    {"hilite", "[attr]\t\t\thighlight your input line", cmd_hilite},
    {"history", "[{number|(expr)}]\tlist/execute commands in history", cmd_history},
    {"host", "[hostname port]]\tset/show address of default host", cmd_host},
	{"identify", "[startact [endact]]\tsend MUME client identification", cmd_identify},
    {"if", "(expr) instr1 [; #else instr2]\tif `expr' is true execute `instr1'\r\n\t\t\t\totherwise execute `instr2'", cmd_if},
    {"in", "[label [(delay) [command]]]\tdelete/list/define delayed labels", cmd_in},
    {"init", "[=[command]]\t\tdefine command to execute on connect to host", cmd_init},
    {"isprompt", "\t\t\trecognize a prompt as such", cmd_isprompt},
    {"key", "name\t\t\texecute the `name' key binding", cmd_key},
    {"keyedit", "editing-name\t\trun a line-editing function", cmd_keyedit},
    {"load", "[filename]\t\tload powwow settings from file", cmd_load},
    {"map", "[-[number]|walksequence]\tshow/clear/edit (auto)map", cmd_map},
    {"mark", "[string[=[attr]]]\t\tdelete/list/define markers", cmd_mark},
    {"movie", "[filename]\t\tbegin/end of movie record to file", cmd_movie},
    {"nice", "[{number|(expr)}[command]]\tset/show priority of new actions/marks", cmd_nice},
    {"option", "[[+|-|=]name]\t\tturn various options", cmd_option},
    {"", "(expr)\t\t\tevaluate expression, trashing result", cmd_eval},
    {"print", "[<|!][text|(expr)]\tprint text/result on screen, appending a \\r\n\r\n\t\t\t\tif no argument, prints value of variable $0", cmd_print},
    {"prompt", "[[<|=|>|%][+|-]name] [{pattern|(expression)} [=[prompt-command]]]\r\n\t\t\t\tdelete/list/define actions on prompts", cmd_prompt},
    {"put", "{text|(expr)}\t\tput text/result of expression in history", cmd_put},
    {"qui", "\t\t\t\tdo nothing", cmd_qui},
    {"quit", "\t\t\t\tquit powwow", cmd_quit},
    {"quote", "[on|off]\t\t\ttoggle verbatim-flag on/off", cmd_quote},
    {"rawsend", "{string|(expr)}\t\tsend raw data to the MUD", cmd_rawsend},
	{"rebind", "name [seq]\t\tchange sequence of a key binding", cmd_rebind},
    {"rebindall", "\t\t\trebind all key bindings", cmd_rebindall},
	{"rebindALL", "\t\t\trebind ALL key bindings, even trivial ones", cmd_rebindALL},
	{"record", "[filename]\t\tbegin/end of record to file", cmd_record},
    {"request", "[editor][prompt][all]\tsend various identification strings", cmd_request},
    {"reset", "<list-name>\t\tclear the whole defined list and reload default", cmd_reset},
    {"retrace", "[number]\t\tretrace the last number steps", cmd_retrace},
    {"save", "[filename]\t\tsave powwow settings to file", cmd_save},
    {"send", "[<|!]{text|(expr)}\teval expression, sending result to the MUD", cmd_send},
	{"separator","set the one character command separator (semi-colon,pipe)",cmd_separator},
    {"setvar", "name[=text|(expr)]\tset/show internal limits and variables", cmd_setvar},
    {"stop", "\t\t\t\tremove all delayed commands from active list", cmd_stop},
    {"time", "\t\t\t\tprint current time and date", cmd_time},
    {"var", "variable [= [<|!]{string|(expr)} ]\twrite result into the variable", cmd_var},
    {"while", "(expr) instr\t\twhile `expr' is true execute `instr'", cmd_while},
    {"write", "[>|!](expr;name)\t\twrite result of expr to `name' file", cmd_write},
    {(char *)0, (char *)0, (function_str)0}
};

void init_cmds () {

	
}
static void cmd_alias  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)
	show_aliases();
    else
	parse_alias(arg);
}

static void cmd_action  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)
	show_actions();
    else
	parse_action(arg, 0);
}

static void cmd_prompt  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)
	show_prompts();
    else
	parse_action(arg, 1);
}

static void cmd_beep  (char *arg)
{
    tty_putc('\007');
}

/*
 * create/list/edit/delete bindings
 */
static void cmd_bind  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)
	show_binds(0);
    else if (!strcmp(arg, "edit"))
	show_binds(1);
    else
	parse_bind(arg);
}

static void cmd_delim  (char *arg)
{
    char buf[BUFSIZE];
    int n;

    arg = skipspace(arg);
    if (!*arg) {
	sprintf(errtext,"#delim: `%s' (%s)\r\n", delim_name[delim_mode], DELIM);
	from_backend(0,errtext,strlen(errtext));
	return;
    }

    arg = split_first_word(buf, BUFSIZE, arg);
    n = 0;
    while (n < DELIM_MODES && strncmp(delim_name[n], buf, strlen(buf)) != 0)
	n++;
    
    if (n >= DELIM_MODES) {
	tty_printf("#delim [normal|program|{custom <chars>}\r\n");
	return;
    }

    if (n == DELIM_CUSTOM) {
	if (!strchr(arg, ' ')) {
	    my_strncpy(buf+1, arg, BUFSIZE-2);
	    *buf = ' ';				/* force ' ' in the delims */
	    arg = buf;
	}
	unescape(arg);
	set_custom_delimeters(arg);
    } else
	delim_mode = n;
}

static void cmd_do  (char *arg)
{
    int type;
    long result;
    
    arg = skipspace(arg);
    if (*arg != '(') {
	tty_printf("#do: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    arg++;
    
    type = evall(&result, &arg);
    if (REAL_ERROR) return;
    
    if (type != TYPE_NUM) {
	printf("#do: ");
	print_error(error=NO_NUM_VALUE_ERROR);
	return;
    }
    
    if (*arg == ')') {          /* skip the ')' */
	if (*++arg == ' ')
	    arg++;
    }
    else {
	printf("#do: ");
	print_error(error=MISSING_PAREN_ERROR);
	return;
    }
    
    if (result >= 0)
	while (!error && result--)
	    (void)parse_instruction(arg, 1, 0, 1);
    else {
	sprintf(errtext,"#do: bogus repeat count `%ld'\r\n", result);
	from_backend(0,errtext,strlen(errtext));
    }
}

static void cmd_hilite  (char *arg)
{
    int attr;
    
    arg = skipspace(arg);
    attr = parse_attributes(arg);
    if (attr == -1) {
	tty_printf("#attribute syntax error.\r\n");
	if (echo_int)
	  show_attr_syntax();
    } else {
	attr_string(attr, edattrbeg, edattrend);

	edattrbg = ATTR(attr) & ATTR_REVERSE ? 1
	    : BACKGROUND(attr) != NO_COLOR || ATTR(attr) & ATTR_BLINK;
	
	if (echo_int) {
	    sprintf(errtext,"#input highlighting is now %so%s%s.\r\n",
		       edattrbeg, (attr == NOATTRCODE) ? "ff" : "n",
		       edattrend);
		from_backend(0,errtext,strlen(errtext));
	}
    }
}

static void cmd_history  (char *arg)
{
    int num = 0;
    long buf;
    
    arg = skipspace(arg);
    
    if (history_done >= MAX_HIST) {
	print_error(error=HISTORY_RECURSION_ERROR);
	return;
    }
    history_done++;
    
    if (*arg == '(') {
	arg++;
	num = evall(&buf, &arg);
	if (!REAL_ERROR && num != TYPE_NUM)
	    error=NO_NUM_VALUE_ERROR;
	if (REAL_ERROR) {
	    tty_printf("#history: \r\n");
	    print_error(error=NO_NUM_VALUE_ERROR);
	    return;
	}
	num = (int)buf;
    } else
	num = atoi(arg);
    
    if (num > 0)
	exe_history(num);  
    else
	show_history(-num);
}

static void cmd_host  (char *arg)
{
    sprintf(errtext,"#Command not allowed in this version\r\n");
	from_backend(0,errtext,strlen(errtext));
    

}

static void cmd_request(char *args)
{
    char *ideditor = "~$#EI\n";

	if (cfg.powwowlocal==0) {return;}

	back->send(ideditor,strlen(ideditor));

	tty_printf("#local edit request done!\r\n");

}

static void cmd_identify  (char *arg)
{
	
	
    edit_start[0] = edit_end[0] = '\0';
    if (*arg) {
        char *p = strchr(arg, ' ');
        if (p) {
            *(p++) = '\0';
            my_strncpy(edit_end, p, BUFSIZE-1);
        }
        my_strncpy(edit_start, arg, BUFSIZE-1);
    }
	
    cmd_request("editor");
	
}

static void cmd_in  (char *arg)
{
    char *name;
    long millisec, buf;
    int type;
    delaynode **p;
    
    arg = skipspace(arg);
    if (!*arg) {
	show_delays();
	return;
    }
    
    arg = first_regular(name = arg, ' ');
    if (*arg)
	*arg++ = 0;
    
    unescape(name);
    
    p = lookup_delay(name, 0);
    if (!*p)  p = lookup_delay(name, 1);
    
    if (!*arg && !*p) {
	sprintf(errtext,"#unknown delay label, cannot show: `%s'\r\n", name);
	from_backend(0,errtext,strlen(errtext));
	return;
    }
    if (!*arg) {
	show_delaynode(*p, 1);
	return;
    }
    if (*arg != '(') {
	printf("#in: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    arg++;    /* skip the '(' */
    
    type = evall(&buf, &arg);
    if (!REAL_ERROR) {
	if (type!=TYPE_NUM)
	    error=NO_NUM_VALUE_ERROR;
	else if (*arg != ')')
	    error=MISSING_PAREN_ERROR;
    }
    if (REAL_ERROR) {
	printf("#in: ");
	print_error(error);
	return;
    }
    
    arg = skipspace(arg+1);
    millisec = buf;
    if (*p && millisec)
	change_delaynode(p, arg, millisec);
    else if (!*p && millisec) {
	if (*arg)
	    new_delaynode(name, arg, millisec);
	else {
	    tty_printf("#cannot create delay label without a command.\r\n");
	}
    } else if (*p && !millisec) {
	if (echo_int) {
	    sprintf(errtext,"#deleting delay label: %s %s\r\n", name, (*p)->command);
		from_backend(0,errtext,strlen(errtext));
	}
	delete_delaynode(p);
    } else {
	sprintf(errtext,"#unknown delay label, cannot delete: `%s'\r\n", name);
	from_backend(0,errtext,strlen(errtext));

    }
}

static void cmd_at  (char *arg)
{
    char *name, *buf = NULL;
    char dayflag=0;
    struct tm *twhen;
    int num, hour = -1, minute = -1, second = -1;
    delaynode **p;
    long millisec;
    ptr pbuf = (ptr)0;
    
    arg = skipspace(arg);
    if (!*arg) {
	show_delays();
	return;
    }
    
    arg = first_regular(name = arg, ' ');
    if (*arg)
	*arg++ = 0;
    
    unescape(name);
    
    p = lookup_delay(name, 0);
    if (!*p)  p = lookup_delay(name, 1);
    
    if (!*arg && !*p) {
	sprintf(errtext,"#unknown delay label, cannot show: `%s'\r\n", name);
	from_backend(0,errtext,strlen(errtext));

	return;
    }
    if (!*arg) {
	show_delaynode(*p, 2);
	return;
    }
    if (*arg != '(') {
	tty_printf("#in: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    arg++;    /* skip the '(' */
    
    (void)evalp(&pbuf, &arg);
    if (REAL_ERROR) {
	print_error(error);
	ptrdel(pbuf);
	return;
    }
    if (pbuf) {
	/* convert time-string into hour, minute, second */
	buf = skipspace(ptrdata(pbuf));
	if (!*buf || !isdigit(*buf)) {
	    tty_printf("#at: ");
	    print_error(error=NO_NUM_VALUE_ERROR);
	    ptrdel(pbuf);
	    return;
	}
	num = atoi(buf);
	second = num % 100;
	minute = (num /= 100) % 100;
	hour   = num / 100;
    }
    if (hour < 0 || hour>23 || minute < 0 || minute>59
	|| second < 0 || second>59) {
	
	sprintf(errtext,"#at: #error: invalid time `%s'\r\n",
	       pbuf && buf ? buf : (char *)"");
	from_backend(0,errtext,strlen(errtext));

	error=OUT_RANGE_ERROR;
	ptrdel(pbuf);
	return;
    }
    ptrdel(pbuf);

    if (*arg == ')') {        /* skip the ')' */
	if (*++arg == ' ')
	    arg++;
    }
    else {
	tty_printf("#at: ");
	print_error(error=MISSING_PAREN_ERROR);
	return;
    }
    
    arg = skipspace(arg);
    update_now();
    twhen = localtime((time_t *)&now); 
    /* put current year, month, day in calendar struct */

    if (hour < twhen->tm_hour || 
	(hour == twhen->tm_hour && 
	 (minute < twhen->tm_min || 
	  (minute == twhen->tm_min &&
	   second <= twhen->tm_sec)))) {
	dayflag = 1;
        /* it is NOT possible to define an #at refering to the past */
    }

    /* if you use a time smaller than the current, it refers to tomorrow */
    
    millisec = (hour - twhen->tm_hour) * 3600 + (minute - twhen->tm_min) * 60 + 
	second - twhen->tm_sec + (dayflag ? 24*60*60 : 0);
    millisec *= CLOCKS_PER_SEC; /* Comparing time with current calendar,
			       we finally got the delay */
    millisec -= now;
    
    if (*p)
	change_delaynode(p, arg, millisec);
    else
	if (*arg)
	new_delaynode(name, arg, millisec);
    else {
	tty_printf("#cannot create delay label without a command.\r\n");
    }
}

static void cmd_init  (char *arg)
{
    arg = skipspace(arg);
    
    if (*arg == '=') {
	if (*++arg) {
	    my_strncpy(initstr, arg, BUFSIZE-1);
	    if (echo_int) {
		sprintf(errtext,"#init: %s\r\n", initstr);
		from_backend(0,errtext,strlen(errtext));

	    }
	} else {
	    *initstr = '\0';
	    if (echo_int) {
		tty_printf("#init cleared.\r\n");
	    }
	}
    } else
	sprintf(inserted_next, "#init =%.*s", BUFSIZE-8, initstr);
}

static void cmd_isprompt  (char *arg)
{
	int i;
	long l;
	arg = skipspace(arg);
	if (*arg == '(') {
	    arg++;
	    i = evall(&l, &arg);
	    if (!REAL_ERROR) {
		if (i!=TYPE_NUM)
		  error=NO_NUM_VALUE_ERROR;
		else if (*arg != ')')
		  error=MISSING_PAREN_ERROR;
	    }
	    if (REAL_ERROR) {
		tty_printf("#isprompt: ");
		print_error(error);
		return;
	    }
	    i = (int)l;
	
	if (i == 0)
	    surely_isprompt = -1;
	else if (i < 0) {
	    if (i > -NUMPARAM && *VAR[-i].str)
		ptrtrunc(prompt->str, surely_isprompt = ptrlen(*VAR[-i].str));
	} else
	    ptrtrunc(prompt->str, surely_isprompt = i);
    }
}

static void cmd_key  (char *arg)
{
    keynode *q=NULL;
    
    arg = skipspace(arg);
    if (!*arg)
	return;
    
    if ((q = *lookup_key(arg)))
	q->funct(q->call_data);
    else {
	sprintf(errtext,"#no such key: `%s'\r\n", arg);
	from_backend(0,errtext,strlen(errtext));

    }
}

static void cmd_keyedit  (char *arg)
{
    int function;
    char *param;
    
    arg = skipspace(arg);
    if (!*arg)
	return;
    
    if ((function = lookup_edit_name(arg, &param)))
	internal_functions[function].funct(param);
    else {
	sprintf(errtext,"#no such editing function: `%s'\r\n", arg);
	from_backend(0,errtext,strlen(errtext));

    }
}

static void cmd_map  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)  /* show map */
	map_show();
    else if (*arg == '-')  /* retrace steps without walking */
	map_retrace(atoi(arg + 1), 0);
    else
	map_walk(arg, 1, 1);
}

static void cmd_retrace  (char *arg)
{
    map_retrace(atoi(arg), 1);
}

static void cmd_mark  (char *arg)
{

    if (!*arg)
	show_marks();
    else
	parse_mark(arg);
}

static void cmd_nice  (char *arg)
{
    int nnice = a_nice;
    arg = skipspace(arg);
    if (!*arg) {
	sprintf(errtext,"#nice: %d\r\n", a_nice);
	from_backend(0,errtext,strlen(errtext));

	return;
    }
    if (isdigit(*arg)) {
	a_nice = 0;
	while (isdigit(*arg)) {
	    a_nice *= 10;
	    a_nice += *arg++ - '0';
	}
    }
    else if (*arg++=='(') {
	long buf;
	int type;

	type = evall(&buf, &arg);
	if (!REAL_ERROR && type!=TYPE_NUM)
	    error=NO_NUM_VALUE_ERROR;
	else if (!REAL_ERROR && *arg++ != ')')
	    error=MISSING_PAREN_ERROR;
	if (REAL_ERROR) {
	    tty_printf("#nice: ");
	    print_error(error);
	    return;
	}
	a_nice = (int)buf;
	if (a_nice<0)
	    a_nice = 0;
    }
    arg = skipspace(arg);
    if (*arg) {
	parse_instruction(arg, 0, 0, 1);
	a_nice = nnice;
    }
}

static void cmd_quote  (char *arg)
{
    arg = skipspace(arg);
    if (!*arg)
	verbatim ^= 1;
    else if (!strcmp(arg, "on"))
	verbatim = 1;
    else if (!strcmp(arg, "off"))
	verbatim = 0;
    if (echo_int) {
        sprintf(errtext,"#%s mode.\r\n", verbatim ? "verbatim" : "normal");
		from_backend(0,errtext,strlen(errtext));
    }
}



static void cmd_reset  (char *arg)
{
    char all = 0;
    arg = skipspace(arg);
    if (!*arg) {
        tty_printf("#reset: must specify one of:\r\n");
        tty_printf(" alias, action, bind, in (or at), mark, prompt, var, all.\r\n");
	return;
    }
    all = !strcmp(arg, "all");
    if (all || !strcmp(arg, "alias")) {
        int n;
	for (n = 0; n < MAX_HASH; n++) {
	    while (aliases[n])
		delete_aliasnode(&aliases[n]);
	}
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "action")) {
        while (actions)
	    delete_actionnode(&actions);
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "bind")) {
        while (keydefs)
	    delete_keynode(&keydefs);
	tty_add_initial_binds();
	tty_add_walk_binds();
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "in") || !strcmp(arg, "at")) {
        while (delays)
	    delete_delaynode(&delays);
        while (dead_delays)
	    delete_delaynode(&dead_delays);
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "mark")) {
        while (markers)
	    delete_marknode(&markers);
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "prompt")) {
        while (prompts)
	    delete_promptnode(&prompts);
	if (!all)
	    return;
    }
    if (all || !strcmp(arg, "var")) {
        int n;
	varnode **first;

	for (n = 0; n < MAX_HASH; n++) {
	    while (named_vars[0][n])
		delete_varnode(&named_vars[0][n], 0);
	    first = &named_vars[1][n];
	    while (*first) {
		if (is_permanent_variable(*first))
		    first = &(*first)->next;
		else	
		    delete_varnode(first, 1);
	    }
	}
	
	for (n = 0; n < NUMVAR; n++) {
	    *var[n].num = 0;
	    ptrdel(*var[n].str);
	    *var[n].str = NULL;
	}
	if (!all)
	    return;
    }
}

static void cmd_stop  (char *arg)
{
    delaynode *dying;
    
    if (delays)
	update_now();
    
    while (delays) {
	dying = delays;
	delays = dying->next;
	dying->when = &now;
	dying->next = dead_delays;
	dead_delays = dying;
    }
    if (echo_int) {
	tty_printf("#all delayed labels are now disabled.\r\n");
    }
}

static void cmd_time  (char *arg)
{
    struct tm *s;
    char buf[BUFSIZE];
    
    update_now();
    s = localtime((time_t *)&now);
    (void)strftime(buf, BUFSIZE - 1, "%a,  %d %b %Y  %H:%M:%S", s);
    sprintf(errtext,"#current time is %s\r\n", buf);
	from_backend(0,errtext,strlen(errtext));
}

static void cmd_emulate  (char *arg)
{
    char kind;
    FILE *fp;
    long start, end, i = 1;
    int len;
    ptr pbuf = (ptr)0;
    
    arg = redirect(arg, &pbuf, &kind, "emulate", 0, &start, &end);
    if (REAL_ERROR || !arg)
	return;
    
    if (kind) {
	char buf[BUFSIZE];
	
	fp = (kind == '!') ? _popen(arg, "r") : fopen(arg, "r");
	if (!fp) {
	    sprintf(errtext,"#emulate: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    print_error(error=SYNTAX_ERROR);
	    ptrdel(pbuf);
	    return;
	}
	status(-1); /* we're pretending we got something from the MUD */
	while (!error && (!start || i<=end) && fgets(buf, BUFSIZE, fp))
	    if (!start || i++>=start)
		process_remote_input(buf, strlen(buf));
	
	if (kind == '!') _pclose(fp); else fclose(fp);
    } else {
	status(-1); /* idem */
	/* WARNING: no guarantee there is space! */
	arg[len = strlen(arg)] = '\r';
	arg[++len] = '\n';
	arg[++len] = '\0';
	process_remote_input(arg, len);
    }
    ptrdel(pbuf);
}

static void cmd_eval  (char *arg)
{
    arg = skipspace(arg);
    if (*arg=='(') {
	arg++;
	(void)evaln(&arg);
	if (*arg != ')') {
	    tty_printf("#(): ");
	    print_error(error=MISSING_PAREN_ERROR);
	}
    }
    else {
	tty_printf("#(): ");
	print_error(error=MISMATCH_PAREN_ERROR);
    }
}

static void cmd_exe  (char *arg)
{
    char kind;
    long start, end, i = 1;
    FILE *fp;
    ptr pbuf = (ptr)0;
    
    arg = redirect(arg, &pbuf, &kind, "exe", 0, &start, &end);
    if (REAL_ERROR || !arg)
	return;

    if (kind) {
	char buf[BUFSIZE];
	
	fp = (kind == '!') ? _popen(arg, "r") : fopen(arg, "r");
	if (!fp) {
	    sprintf(errtext,"#exe: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    error = SYNTAX_ERROR;
	    ptrdel(pbuf);
	    return;
	}
	while (!error && (!start || i<=end) && fgets(buf, BUFSIZE, fp))
	    if (!start || i++>=start) {
		buf[strlen(buf)-1] = '\0';
		parse_user_input(buf, 0);
	    }
	
	if (kind == '!') _pclose(fp); else fclose(fp);
    } else
	parse_user_input(arg, 0);
    ptrdel(pbuf);
}

static void cmd_print  (char *arg)
{
    char kind;
    long start, end, i = 1;
    FILE *fp;
    ptr pbuf = (ptr)0;
    
    //clear_input_line(opt_compact);
    

    if (!*arg) {
	smart_print(*VAR[0].str ? ptrdata(*VAR[0].str) : (char *)"", 1);
	return;
    }
    
    arg = redirect(arg, &pbuf, &kind, "print", 1, &start, &end);
    if (REAL_ERROR || !arg)
	return;

    if (kind) {
	char buf[BUFSIZE];
	fp = (kind == '!') ? _popen(arg, "r") : fopen(arg, "r");
	if (!fp) {
	    sprintf(errtext,"#print: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    error=SYNTAX_ERROR;
	    ptrdel(pbuf);
	    return;
	}
	while (!error && (!start || i <= end) && fgets(buf, BUFSIZE, fp))
	    if (!start || i++>=start)
		tty_printf(buf);
	tty_printf("\r\n");

	
	if (kind == '!') _pclose(fp); else fclose(fp);
    } else
	smart_print(arg, 1);
    ptrdel(pbuf);
}

static void cmd_send  (char *arg)
{
	
    char *newline, kind;
    long start, end, i = 1;
    FILE *fp;
    ptr pbuf = (ptr)0;
    
    arg = redirect(arg, &pbuf, &kind, "send", 0, &start, &end);
    if (REAL_ERROR ||!arg)
	return;
    
    if (kind) {
	char buf[BUFSIZE];
	fp = (kind == '!') ? _popen(arg, "r") : fopen(arg, "r");
	if (!fp) {
	    sprintf(errtext,"#send: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    error = SYNTAX_ERROR;
	    ptrdel(pbuf);
	    return;
	}
	while (!error && (!start || i<=end) && fgets(buf, BUFSIZE, fp)) {
	    if ((newline = strchr(buf, '\r\n')))
		*newline = '\0';
	    
	    if (!start || i++>=start) {
		if (echo_ext) {
		    sprintf(errtext,"\r\n[%s]\r\n", buf);
			from_backend(0,errtext,strlen(errtext));
		}
			back->send(buf,strlen(buf));

	    }
	}
	if (kind == '!') _pclose(fp); else fclose(fp);
    } else {
	if (echo_ext) {
	    sprintf(errtext,"[%s]\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	}
	back->send(arg,strlen(arg));
    }
    ptrdel(pbuf);
	back->send("\n",1);
}

static void cmd_rawsend  (char *arg)
{
	// send is basically rawsend in PowTTY
	cmd_send(arg);
}




static void cmd_write  (char *arg)
{
    ptr p1 = (ptr)0, p2 = (ptr)0;
    char *tmp = skipspace(arg), kind;
    FILE *fp;
    
    kind = *tmp;
    if (kind = '!' || kind == '>')
	arg = ++tmp;
    else
	kind = 0;
    
    if (*tmp=='(') {
	arg = tmp + 1;
	(void)evalp(&p1, &arg);
	if (REAL_ERROR)
	    goto write_cleanup;
	
	if (*arg == CMDSEP) {
	    arg++;
	    (void)evalp(&p2, &arg);
	    if (!REAL_ERROR && !p2)
		error = NO_STRING_ERROR;
	    if (REAL_ERROR)
		goto write_cleanup;
	} else {
	    tty_printf("#write: ");
	    error=SYNTAX_ERROR;
	    goto write_cleanup;
	}
	if (*arg != ')') {
	    tty_printf("#write: ");
	    error=MISSING_PAREN_ERROR;
	    goto write_cleanup;
	}
	arg = ptrdata(p2);
	
	fp = (kind == '!') ? _popen(arg, "w") : fopen(arg, kind ? "w" : "a");
	if (!fp) {
	    sprintf(errtext,"#write: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    error=SYNTAX_ERROR;
	    goto write_cleanup2;
	}
	fprintf(fp, "%s\r\n", p1 ? ptrdata(p1) : (char *)"");
	fflush(fp);
	if (kind == '!') _pclose(fp); else fclose(fp);
    } else {
	printf("#write: ");
	error=MISMATCH_PAREN_ERROR;
    }

write_cleanup:
    if (REAL_ERROR)
	print_error(error);
write_cleanup2:
    ptrdel(p1);
    ptrdel(p2);
}

static void cmd_var  (char *arg)
{
    char *buf, *expr, *tmp, kind, type, right = 0, deleting = 0;
    varnode **p_named_var = NULL, *named_var = NULL;
    FILE *fp;
    long start, end, i = 1;
    int len, idx;
    ptr pbuf = (ptr)0;
    
    arg = skipspace(arg);
    expr = first_regular(arg, '=');
    
    if (*expr) {
	*expr++ = '\0';     /* skip the = */
	if (!*expr)
	    deleting = 1;
	else {
	    right = 1;
	    if (*expr == ' ')
		expr++;
	}
    }
    
    if (*arg == '$')
	type = TYPE_TXT_VAR;
    else if (*arg == '@')
	type = TYPE_NUM_VAR;
    else if (*arg) {
	print_error(error=INVALID_NAME_ERROR);
	return;
    } else {
	show_vars();
	return;
    }
    
    kind = *++arg;
    if (isalpha(kind) || kind == '_') {
	/* found a named variable */
	tmp = arg;
	while (*tmp && (isalnum(*tmp) || *tmp == '_'))
	    tmp++;
	if (*tmp) {
	    print_error(error=INVALID_NAME_ERROR);
	    return;
	}
	kind = type==TYPE_TXT_VAR ? 1 : 0;
	p_named_var = lookup_varnode(arg, kind);
	if (!*p_named_var) {
	    /* it doesn't (yet) exist */
	    if (!deleting) {
		/* so create it */
		named_var = add_varnode(arg, kind);
		if (REAL_ERROR)
		    return;
		if (echo_int) {
		    sprintf(errtext,"#new variable: `%s'\r\n", arg - 1);
			from_backend(0,errtext,strlen(errtext));
		}
	    } else {
		print_error(error=UNDEFINED_VARIABLE_ERROR);
		return;
	    }
	} else
	    /* it exists, hold on */
	    named_var = *p_named_var;
	
	idx = named_var->index;
    } else {
	/* not a named variable, may be
	 * an unnamed variable or an expression */
	kind = type==TYPE_TXT_VAR ? 1 : 0;
	tmp = skipspace(arg);
	if (*tmp == '(') {
	    /* an expression */
	    arg = tmp+1;
	    idx = evalp(&pbuf, &arg);
	    if (!REAL_ERROR && idx != TYPE_TXT)
		error=NO_STRING_ERROR;
	    if (REAL_ERROR) {
		tty_printf("#var: ");
		print_error(error);
		ptrdel(pbuf);
		return;
	    }
	    if (pbuf)
		buf = ptrdata(pbuf);
	    else
		buf = "";
	    if (isdigit(*buf)) {
		idx = atoi(buf);
		if (idx < -NUMVAR || idx >= NUMPARAM) {
		    print_error(error=OUT_RANGE_ERROR);
		    ptrdel(pbuf);
		    return;
		}
	    } else {
		if (!(named_var = *lookup_varnode(buf, kind))) {
		    if (!deleting) {
			named_var = add_varnode(buf, kind);
			if (REAL_ERROR) {
			    print_error(error);
			    ptrdel(pbuf);
			    return;
			}
			if (echo_int) {
			    sprintf(errtext,"#new variable: %c%s\r\n", kind
				       ? '$' : '@', buf);
				from_backend(0,errtext,strlen(errtext));
			}
		    } else {
			print_error(error=UNDEFINED_VARIABLE_ERROR);
			ptrdel(pbuf);
			return;
		    }
		}
		idx = named_var->index;
	    }
	} else {
	    /* an unnamed var */
	    long buf2;
	    
	    idx = evall(&buf2, &arg);
	    if (!REAL_ERROR && idx != TYPE_NUM)
		error=NO_STRING_ERROR;
	    if (REAL_ERROR) {
		printf("#var: ");
		print_error(error);
		return;
	    }
	    idx = (int)buf2;
	    if (idx < -NUMVAR || idx >= NUMPARAM) {
		print_error(error=OUT_RANGE_ERROR);
		return;
	    }
	    /* ok, it's an unnamed var */
	}
    }

    
    if (type == TYPE_TXT_VAR && right && !*VAR[idx].str) {
	/* create it */
	*VAR[idx].str = ptrnew(PARAMLEN);
	if (MEM_ERROR) {
	    print_error(error);
	    ptrdel(pbuf);
	    return;
	}
    }

    if (deleting) {
	/* R.I.P. named variables */
	if (named_var) {
	    if (is_permanent_variable(named_var)) {
		sprintf(errtext,"#cannot delete variable: `%s'\r\n", arg - 1);
		from_backend(0,errtext,strlen(errtext));
	    } else {
		delete_varnode(p_named_var, kind);
		if (echo_int) {
		    sprintf(errtext,"#deleted variable: `%s'\r\n", arg - 1);
			from_backend(0,errtext,strlen(errtext));
		}
	    }
	} else if ((type = TYPE_TXT_VAR)) {
	/* R.I.P. unnamed variables */	    
	    if (*VAR[idx].str) {
		if (idx < 0) {
		    ptrdel(*VAR[idx].str);
		    *VAR[idx].str = 0;
		} else
		    ptrzero(*VAR[idx].str);
	    }
	} else
	    *VAR[idx].num = 0;
	ptrdel(pbuf);
	return;
    } else if (!right) {
	/* no right-hand expression, just show */
	if (named_var) {
	    if (type == TYPE_TXT_VAR) {
		pbuf = ptrescape(pbuf, *VAR[idx].str, 0);
		if (REAL_ERROR) {
		    print_error(error);
		    ptrdel(pbuf);
		    return;
		}
		sprintf(inserted_next, "#($%.*s = \"%.*s\")",
			BUFSIZE - 10, named_var->name,
			BUFSIZE - (int)strlen(named_var->name) - 10,
			pbuf ? ptrdata(pbuf) : (char *)"");
	    } else {
		sprintf(inserted_next, "#(@%.*s = %ld)",
			BUFSIZE - 8, named_var->name,
			*VAR[idx].num);
	    }
	} else {
	    if (type == TYPE_TXT_VAR) {
		pbuf = ptrescape(pbuf, *VAR[idx].str, 0);
		sprintf(inserted_next, "#($%d = \"%.*s\")", idx,
			BUFSIZE - INTLEN - 10,
			pbuf ? ptrdata(pbuf) : (char *)"");
	    } else {
		sprintf(inserted_next, "#(@%d = %ld)", idx,
			*VAR[idx].num);
	    }
	}
	ptrdel(pbuf);
	return;
    }
    
    /* only case left: assign a value to a variable */
    arg = redirect(expr, &pbuf, &kind, "var", 1, &start, &end);
    if (REAL_ERROR || !arg)
	return;

    if (kind) {
	char buf2[BUFSIZE];
	fp = (kind == '!') ? _popen(arg, "r") : fopen(arg, "r");
	if (!fp) {
	    sprintf(errtext,"#var: #error opening `%s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
	    error=SYNTAX_ERROR;
	    ptrdel(pbuf);
	    return;
	}
	len = 0;
	i = 1;
	while (!error && (!start || i<=end) && fgets(buf2+len, BUFSIZE-len, fp))
	    if (!start || i++>=start)
		len += strlen(buf2 + len);
	
	if (kind == '!') _pclose(fp); else fclose(fp);
	if (len>PARAMLEN)
	    len = PARAMLEN;
	buf2[len] = '\0';
	arg = buf2;
    }
    
    if (type == TYPE_NUM_VAR) {
	arg = skipspace(arg);
	type = 1;
	len = 0;
	
	if (*arg == '-')
	    arg++, type = -1;
	else if (*arg == '+')
	    arg++;
	
	if (isdigit(kind=*arg)) while (isdigit(kind)) {
	    len*=10;
	    len+=(kind-'0');
	    kind=*++arg;
	}
	else {
	    tty_printf("#var: ");
	    print_error(error=NO_NUM_VALUE_ERROR);
	}
	*VAR[idx].num = len * type;
    }
    else {
	*VAR[idx].str = ptrmcpy(*VAR[idx].str, arg, strlen(arg));
	if (MEM_ERROR)
	    print_error(error);
    }
    ptrdel(pbuf);
}

static void cmd_setvar  (char *arg)
{
    char *name;
    int i, func = 0; /* show */
    long buf;
    
    name = arg = skipspace(arg);
    arg = first_regular(arg, '=');
    if (*arg) {
	*arg++ = '\0';
	if (*arg) {
	    func = 1; /* set */
	    if (*arg == '(') {
		arg++;
		i = evall(&buf, &arg);
		if (!REAL_ERROR && i != TYPE_NUM)
		    error=NO_NUM_VALUE_ERROR;
		else if (!REAL_ERROR && *arg != ')')
		    error=MISSING_PAREN_ERROR;
	    } else
		buf = strtol(arg, NULL, 0);
	} else
	    buf = 0;
    
	if (REAL_ERROR) {
	    tty_printf("#setvar: ");
	    print_error(error);
	    return;
	}
    }
    
	tty_printf("\r\n");

    i = strlen(name);
    if (i && !strncmp(name, "timer", i)) {
	time_t t;
	update_now();
	if (func == 0)
	    sprintf(inserted_next, "#setvar timer=%ld",
		    now-ref_time);
	else {
	    t=  (-buf);
	    ref_time =now;
	    add_vtime(&ref_time, &t);
	}
    }
    else if (i && !strncmp(name, "lines", i)) {
	if (func == 0)
	    sprintf(inserted_next, "#setvar lines=%d", lines);
	else {
	    if (buf > 0)
		lines = (int)buf;
	    if (echo_int) {
		sprintf(errtext,"#setvar: lines=%d\r\n", lines);
		from_backend(0,errtext,strlen(errtext));
	    }
	}
    }
    else if (i && !strncmp(name, "mem", i)) {
	if (func == 0)
	    sprintf(inserted_next, "#setvar mem=%d", limit_mem);
	else {
	    if (buf == 0 || buf >= PARAMLEN)
		limit_mem = buf <= INT_MAX ? (int)buf : INT_MAX;
	    if (echo_int) {
		sprintf(errtext,"#setvar: mem=%d%s\r\n", limit_mem,
		       limit_mem ? "" : " (unlimited)");
		from_backend(0,errtext,strlen(errtext));
	    }
	}
    }
    else if (i && !strncmp(name, "buffer", i)) {
	if (func == 0)
	    sprintf(inserted_next, "#setvar buffer=%d", log_getsize());
	else
	    log_resize(buf);
    } else {
	update_now();
	sprintf(errtext,"#setvar buffer=%d#setvar lines=%d#setvar mem=%d#setvar timer=%ld\r\n",
	       log_getsize(), lines, limit_mem, now-ref_time);
	from_backend(0,errtext,strlen(errtext));
    }
}

static void cmd_if  (char *arg)
{
    long buf;
    int type;
    
    arg = skipspace(arg);
    if (*arg!='(') {
	tty_printf("#if: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    arg++;  /* skip the '(' */
    
    type = evall(&buf, &arg);
    if (!REAL_ERROR) {
	if (type!=TYPE_NUM)
	    error=NO_NUM_VALUE_ERROR;
	if (*arg != ')')
	    error=MISSING_PAREN_ERROR;
	else {              /* skip the ')' */
	    if (*++arg == ' ')
		arg++;
	}
    }
    if (REAL_ERROR) {
	tty_printf("#if: ");
	print_error(error);
	return;
    }
    
    if (buf)
	(void)parse_instruction(arg, 0, 0, 1);
    else {
	arg = get_next_instr(arg);
	if (!strncmp(arg = skipspace(arg), "#else ", 6))
	    (void)parse_instruction(arg + 6, 0, 0, 1);
    }
}

static void cmd_for  (char *arg)
{
    int type = TYPE_NUM, loop=MAX_LOOP;
    long buf;
    char *check, *tmp, *increm = 0;
    
    arg = skipspace(arg);
    if (*arg != '(') {
	tty_printf("#for: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    push_params();
    if (REAL_ERROR)
	return;
    
    arg = skipspace(arg + 1);    /* skip the '(' */
    if (*arg != CMDSEP)
	(void)evaln(&arg);       /* execute <init> */
    
    check = arg + 1;
    
    if (REAL_ERROR)
	;
    else if (*arg != CMDSEP) {
	printf("#for: ");
	print_error(error=MISSING_SEPARATOR_ERROR);
    }
    else while (!error && loop
		&& (increm=check, (type = evall(&buf, &increm)) == TYPE_NUM
		    && !error && *increm == CMDSEP && buf)) {
	
	tmp = first_regular(increm + 1, ')');
	if (*tmp)
	    (void)parse_instruction(tmp + 1, 1, 1, 1);
	else {
	    tty_printf("#for: ");
	    print_error(error=MISSING_PAREN_ERROR);
	}
	
	if (!error) {
	    tmp = increm + 1;
	    if (*tmp != ')')
		(void)evaln(&tmp);
	}
	
	loop--;
    }
    if (REAL_ERROR)
	;
    else if (increm && *increm != CMDSEP)
	error=MISSING_SEPARATOR_ERROR;
    else if (!loop)
	error=MAX_LOOP_ERROR;
    else if (type != TYPE_NUM)
	error=NO_NUM_VALUE_ERROR;
    if (REAL_ERROR) {
	printf("#for: ");
	print_error(error);
    }
    if (error!=DYN_STACK_UND_ERROR && error!=DYN_STACK_OV_ERROR)
	pop_params();
}

static void cmd_while  (char *arg)
{
    int type = TYPE_NUM, loop=MAX_LOOP;
    long buf;
    char *check, *tmp;
    
    arg = skipspace(arg);
    if (!*arg) {
	tty_printf("#while: ");
	print_error(error=MISMATCH_PAREN_ERROR);
	return;
    }
    push_params();
    
    check = ++arg;   /* skip the '(' */
    while (!error && loop
	   && (arg=check, (type = evall(&buf, &arg)) == TYPE_NUM &&
	       !error && *arg == ')' && buf)) {
	
	if (*(tmp = arg + 1) == ' ')          /* skip the ')' */
	    tmp++;
	if (*tmp)
	    (void)parse_instruction(tmp, 1, 1, 1);
	loop--;
    }
    if (REAL_ERROR)
	;
    else if (*arg != ')')
	error=MISSING_PAREN_ERROR;
    else if (!loop)
	error=MAX_LOOP_ERROR;
    else if (type != TYPE_NUM)
	error=NO_NUM_VALUE_ERROR;
    if (REAL_ERROR) {
	tty_printf("#while: ");
	print_error(error);
    }
    if (error!=DYN_STACK_UND_ERROR && error!=DYN_STACK_OV_ERROR)
	pop_params();
}

static void cmd_capture  (char *myarg)
{


    if (!*myarg) {
        if (capturefile) {
	    log_flush();
            fclose(capturefile);
            capturefile = NULL;
            if (echo_int) {
			tty_printf("#end of capture to file.\r\n");
            }
        } else {
            tty_printf("#capture to what file?\r\n");
        }
    } else {
			char *arg;
			int x=strlen(myarg)+5 +strlen(cfg.powwowdir);
			arg=malloc(x);
		    myarg = skipspace(myarg);
			sprintf(arg,"%s\\%s",cfg.powwowdir,myarg,".txt");

		   if (capturefile) {
			    tty_printf("#capture already active.\r\n");
			} else {
				if ((capturefile = fopen(arg, "w")) == NULL) {
					sprintf(errtext,"#error writing file `%s'\r\n", arg);
					from_backend(0,errtext,strlen(errtext));
				} else if (echo_int) {
					sprintf(errtext,"#capture to `%s' active, `#capture' ends.\r\n", arg);
					from_backend(0,errtext,strlen(errtext));
				}
			}
			free(arg);

    }
} 

static void cmd_movie  (char *arg)
{
    arg = skipspace(arg);
    
    if (!*arg) {
        if (moviefile) {
	    log_flush();
            fclose(moviefile);
            moviefile = NULL;
            if (echo_int) {
		tty_printf("#end of movie to file.\r\n");
            }
        } else {
            tty_printf("#movie to what file?\r\n");
        }
    } else {
        if (moviefile) {
            tty_printf("#movie already active.\r\n");
        } else {
            if ((moviefile = fopen(arg, "w")) == NULL) {
                sprintf(errtext,"#error writing file `%s'\r\n", arg);
				from_backend(0,errtext,strlen(errtext));
            } else {
		if (echo_int) {
		    sprintf(errtext,"#movie to `%s' active, `#movie' ends.\r\n", arg);
			from_backend(0,errtext,strlen(errtext));
		}
		update_now();
		movie_last = now;
		log_clearsleep();
            }
        }
    }
}

static void cmd_record  (char *arg)
{
    arg = skipspace(arg);
    
    if (!*arg) {
        if (recordfile) {
            fclose(recordfile);
            recordfile = NULL;
            if (echo_int) {
		printf("#end of record to file.\r\n");
            }
        } else {
            tty_printf("#record to what file?\r\n");
        }
    } else {
        if (recordfile) {
            tty_printf("#record already active.\r\n");
        } else {
            if ((recordfile = fopen(arg, "w")) == NULL) {
                sprintf(errtext,"#error writing file `%s'\r\n", arg);
				from_backend(0,errtext,strlen(errtext));
            } else if (echo_int) {
                sprintf(errtext,"#record to `%s' active, `#record' ends.\r\n", arg);
				from_backend(0,errtext,strlen(errtext));
            }
        }
    }
}

static void cmd_edit  (char *arg)
{
	
    editsess *sp;
    tty_printf("\r\n");
    if (edit_sess) {
        for (sp = edit_sess; sp; sp = sp->next) {
				sprintf(errtext,"# %s (%u)\r\n", sp->descr, sp->key);
				from_backend(0,errtext,strlen(errtext));
		}
    } else {
	tty_printf("#no active editors.\r\n");
    }
	
}

static void cmd_cancel  (char *arg)
{
	
    editsess *sp;
    
    if (!edit_sess) {
        tty_printf("#no editing sessions to cancel.\r\n");
    } else {
        if (*arg) {
            for (sp = edit_sess; sp; sp = sp->next)
		if (strtoul(arg, NULL, 10) == sp->key) {
		    cancel_edit(sp);
		    break;
		}
            if (!sp) {
                sprintf(errtext,"#unknown editing session %d\r\n", atoi(arg));
				from_backend(0,errtext,strlen(errtext));
            }
        } else {
            if (edit_sess->next) {
                tty_printf("#several editing sessions active, use #cancel <number>\r\n");
            } else
		cancel_edit(edit_sess);
        }
    }
	
}


#ifndef CLOCKS_PER_SEC
#  define CLOCKS_PER_SEC uSEC_PER_SEC
#endif
/* hope it works.... */

static void cmd_cpu  (char *arg)
{
    float f, l;
    update_now();
    f = (float)((cpu_clock = clock()) - start_clock) / (float)CLOCKS_PER_SEC;
    l = (float)(now-start_time) / (float)CLOCKS_PER_SEC;
    sprintf(errtext,"#CPU time used: %.3f sec. (%.2f%%)\r\n", f,
	       (l > 0 && l > f) ? f * 100.0 / l : 100.0);
	from_backend(0,errtext,strlen(errtext));
}

static void cmd_qui  (char *arg)
{
    sprintf(errtext,"#you have to write '#quit' - no less, to quit!\r\n");
	from_backend(0,errtext,strlen(errtext));
}

static void cmd_quit  (char *arg)
{
    if (*arg) { /* no skipspace() here! */
	tty_printf("#quit: spurious argument?\r\n");
    } else
	exit_powwow();
}

char *full_options_string = "#option %cexit %chistory %cwords %ccompact %cdebug %cecho %cinfo %ckeyecho %cspeedwalk#option %cwrap %cautoprint %creprint %csendsize %cautoclear%s";

static void cmd_option  (char *arg)
{
    if (*(arg = skipspace(arg))) {
	int len, i, count = 0;
	static char *str[] = { "exit", "history", "words",
		"compact", "debug", "echo", "info", "keyecho",
		"speedwalk", "wrap", "autoprint", "reprint",
		"sendsize", "autoclear", 0 };
	static char *varptr[] = { &opt_exit, &opt_history, &opt_words,
		&opt_compact, &opt_debug, &echo_ext, &echo_int, &echo_key,
		&opt_speedwalk, &opt_wrap, &opt_autoprint, &opt_reprint,
		&opt_sendsize, &opt_autoclear, 0 };
	enum { MODE_ON, MODE_OFF, MODE_TOGGLE, MODE_REP } mode;
	char buf[BUFSIZE], *p, *varp, c;
	while ((arg = skipspace(split_first_word(buf, BUFSIZE, arg))), *buf) {
	    c = *(p = buf);
	    switch (c) {
	     case '=': mode = MODE_REP; p++; break;
	     case '+': mode = MODE_ON;  p++; break;
	     case '-': mode = MODE_OFF; p++; break;
	     default:  mode = MODE_TOGGLE;     break;
	    }
	    len = strlen(p);
	    varp = 0;
	    count++;
	    for (i=0; str[i]; i++) {
		if (strncmp(str[i], p, len) == 0) {
		    varp = varptr[i];
		    break;
		}
	    }
	    if (!str[i]) {
		if (strncmp("none", p, len) == 0)
		    continue;
		else {
		    sprintf(errtext,"#syntax: #option [[+|-|=]<name>]\t\twhere <name> is one of:\r\n\
exit history words compact debug echo info\r\n\
keyecho speedwalk wrap autoprint reprint sendsize autoclear\r\n");
			from_backend(0,errtext,strlen(errtext));
		    return;
		}
	    }
	    if (varp) {		
		switch (mode) {
		 case MODE_REP:
		    sprintf(inserted_next, "#option  %c", *varp ? '+' : '-');
		    strcat(inserted_next, p);
		    break;
		 case MODE_ON:  *varp  = 1; break;
		 case MODE_OFF: *varp  = 0; break;
		 default:       *varp ^= 1; break;
		}
		/*
		 * reset the reprint buffer if changing its status
		 */
		if (varp == &opt_reprint)
		    reprint_clear();

		/* as above, but always print status if
		 * `#option info' alone was typed */
		if (mode != MODE_REP && !*arg && count==1 &&
		    (echo_int || (mode == MODE_TOGGLE && varp==&echo_int))) {
		    sprintf(errtext,"#option %s is now o%s.\r\n", str[i],
			      *varp ? "n" : "ff");
			from_backend(0,errtext,strlen(errtext));
		}
	    }
	}
    } else {
	printf(full_options_string,
		opt_exit  ? '+' : '-',
		opt_history ? '+' : '-',
		opt_words ? '+' : '-',
		opt_compact ? '+' : '-',
		opt_debug ? '+' : '-',
		echo_ext  ? '+' : '-',
		echo_int  ? '+' : '-',
		echo_key  ? '+' : '-',
		opt_speedwalk ? '+' : '-',
		opt_wrap  ? '+' : '-',
		opt_autoprint ? '+' : '-',
		opt_reprint ? '+' : '-',
		opt_sendsize ? '+' : '-',
		opt_autoclear ? '+' : '-',
		"\r\n");
    }
}

static void cmd_file  (char *arg)
{
    arg = skipspace(arg);
    if (*arg == '=') {
	set_deffile(++arg);
	if (echo_int) {
	    if (*arg) {
		sprintf(errtext,"#save-file set to `%s'\r\n", deffile);
		from_backend(0,errtext,strlen(errtext));
	    } else {
		tty_printf("#save-file is now undefined.\r\n");
	    }
	}
    } else if (*deffile) {
	sprintf(inserted_next, "#file =%.*s", BUFSIZE-8, deffile);
    } else {			  
	printf("#save-file not defined.\r\n");
    }
}

static void cmd_save  (char *arg)
{ 
    arg = skipspace(arg);
    if (*arg) {
	set_deffile(arg);
	if (echo_int) {
	    sprintf(errtext,"#save-file set to `%s'\r\n", deffile);
		from_backend(0,errtext,strlen(errtext));
	}
    } else if (!*deffile) {
	tty_printf("#save-file not defined.\r\n");
	return;
    }
    
    if (*deffile && pow_save_settings() > 0 && echo_int) {
	tty_printf("#settings saved to file.\r\n");
    }
}

static void cmd_load  (char *arg)
{
    int res;
    
    arg = skipspace(arg);
    if (*arg) {
	set_deffile(arg);
	if (echo_int) {
	    sprintf(errtext,"#save-file set to `%s'\r\n", deffile);
		from_backend(0,errtext,strlen(errtext));
	}
    }
    else if (!*deffile) {
	printf("#save-file not defined.\r\n");
	return;
    }
    
    res = read_settings();
    
    if (res > 0) {
	/* success */
	if (echo_int) {
	    tty_printf("#settings loaded from file.\r\n");
	}
    } else if (res < 0) {
	/* critical error */
	while (keydefs)
	    delete_keynode(&keydefs);
	tty_add_initial_binds();
	tty_add_walk_binds();
	limit_mem = 1048576;
	printf("#emergency loaded default settings.\r\n");
    }
}

static char *trivial_eval  (ptr *pbuf, char *arg, char *name)
{
    char *tmp = skipspace(arg);
    
    if (!pbuf)
	return NULL;
    
    if (*tmp=='(') {
	arg = tmp + 1;
	(void)evalp(pbuf, &arg);
	if (!REAL_ERROR && *arg != ')')
	    error=MISSING_PAREN_ERROR;
	if (REAL_ERROR) {
	    sprintf(errtext,"#%s: ", name);
		from_backend(0,errtext,strlen(errtext));
	    print_error(error);
	    return NULL;
	}
	if (*pbuf)
	    arg = ptrdata(*pbuf);
	else
	    arg = "";
    }
    else
	unescape(arg);
    
    return arg;
}

static void cmd_add  (char *arg)
{
    ptr pbuf = (ptr)0;
    char buf[BUFSIZE];
    
    arg = trivial_eval(&pbuf, arg, "add");
    if (!REAL_ERROR)
	while (*arg) {
	    arg = split_first_word(buf, BUFSIZE, arg);
	    if (strlen(buf) >= MIN_WORDLEN)
		put_word(buf);
	}
    ptrdel(pbuf);
}

static void cmd_put  (char *arg)
{
    ptr pbuf = (ptr)0;
    arg = trivial_eval(&pbuf, arg, "put");
    if (!REAL_ERROR && *arg)
	put_history(arg);
    ptrdel(pbuf);
}


static void cmd_rebind  (char *arg)
{
    parse_rebind(arg);
}

static void cmd_rebindall (char *arg)
{
    keynode *kp;
    char *seq;
    
    for (kp = keydefs; kp; kp = kp->next) {
	seq = kp->sequence;
	if (kp->seqlen == 1 && seq[0] < ' ')
	    ;
	else if (kp->seqlen == 2 && seq[0] == '\033' && isalnum(seq[1]))
	    ;
	else {
	    parse_rebind(kp->name);
	    if (error)
		break;
	}
    }
}

static void cmd_rebindALL  (char *arg)
{
    keynode *kp;
    
    for (kp = keydefs; kp; kp = kp->next) {
	parse_rebind(kp->name);
	if (error)
	    break;
    }
}


static void cmd_separator (char *arg) {

	if (*arg=='\0') {
		sprintf(errtext,"#command separator=%c\r\n",CMDSEP);
		from_backend(0,errtext,strlen(errtext));
		return;
	}

	if ((*arg != '|') && (*arg != ';') && (*arg != '~')) {
		tty_printf("#error: command separator must be one of: | ; ~\r\n");
		return;
	}

	CMDSEP=arg[0];

}