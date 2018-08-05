/*
 *  cmd2.c  --  back-end for the various #commands
 *
 *  (created: Massimiliano Ghilardi (Cosmos), Aug 14th, 1998)
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
#include <errno.h>

#include "malloc.h"
#include "regex.h"

//int select();


#include "powdefines.h"
#include "powutils.h"
#include "powmain.h"
#include "powlist.h"
#include "poweval.h"
#include "powedit.h"
#include "powtty.h"
#include "powcmd2.h"
#include "powhook.h"
#include "putty.h"

/* anyone knows if ANSI 6429 talks about more than 8 colors? */
static char *colornames[] = {
    "black", "red", "green", "yellow", "blue", "magenta", "cyan", "white",
    "BLACK", "RED", "GREEN", "YELLOW", "BLUE", "MAGENTA", "CYAN", "WHITE", "none"
};

static char *tmpcommand;
static char *tmpfuncname;
static keynode *tmpkeynode;
/*
 * show defined aliases
 */
void show_aliases  (void)
{
    aliasnode *p;
    char buf[BUFSIZE];
    
    sprintf(errtext,"#%s alias%s defined%c\r\n", sortedaliases ? "the following" : "no",
	       (sortedaliases && !sortedaliases->snext) ? " is" : "es are",
	       sortedaliases ? ':' : '.');
    reverse_sortedlist((sortednode **)&sortedaliases);
	from_backend(0,errtext,strlen(errtext));
    for (p = sortedaliases; p; p = p->snext) {
	escape_specials(buf, p->name);
	sprintf(errtext,"#alias %s=%s\r\n", buf, p->subst);
	from_backend(0,errtext,strlen(errtext));

    }
    reverse_sortedlist((sortednode **)&sortedaliases);
}

/*
 * check if an alias name contains dangerous things.
 * return 1 if illegal name (and print reason).
 * if valid, print a warning for unbalanced () {} and ""
 */
static int check_alias  (char * name)
{
    char *p = name, c;
    enum { NORM, ESCAPE } state = NORM;
    int quotes=0, paren=0, braces=0, ok = 1;

    if (!*p) {
	printf("#illegal alias name: is empty!\r\n");
	error = INVALID_NAME_ERROR;
	return 1;
    }
    if (*p == '{') {
	printf("#illegal beginning '{' in alias name: `%s'\r\n", name);
	error = INVALID_NAME_ERROR;
	return 1;
    }
    if (strchr(name, ' ')) {
	printf("#illegal spaces in alias name: `%s'\r\n", name);
	error = INVALID_NAME_ERROR;
	return 1;
    }

    for (; ok && (c = *p); p++) switch (state) {
      case NORM:
	if (c == ESC)
	    state = ESCAPE;
	else if (quotes) {
	    if (c == '\"')
		quotes = 0;
	}
	else if (c == '\"')
	    quotes = 1;
	else if (c == ')')
	    paren--;
	else if (c == '(')
	    paren++;
	else if (c == '}')
	    braces--;
	else if (c == '{')
	    braces++;
	else if (c == CMDSEP && !paren && !braces)
	    ok = 0;
	break;
      case ESCAPE:
	if (c == ESC)
	    state = ESCAPE;
	else /* if (c == ESC2 || c != ESC2) */
	    state = NORM;
      default:
	break;
    }
	
    if (!ok) {
	printf("#illegal non-escaped `;' in alias name: `%s'\r\n", name);
	error = INVALID_NAME_ERROR;
	return 1;
    }
	
    if (quotes || paren || braces) {
	printf("#warning: unbalanced%s%s%s in alias name `%s' may cause problems\r\n",
	       quotes ? " \"\"" : "", paren ? " ()" : "", braces ? " {}" : "", name);
    }

    return 0;
}


/*
 * parse the #alias command
 */
void parse_alias  (char * str)
{
    char *left, *right;
    aliasnode **np, *p;
    
    left = str = skipspace(str);
    
    str = first_valid(str, '=');
    
    if (*str == '=') {
        *str = '\0';
        right = ++str;
        unescape(left);
	if (check_alias(left))
	    return;
        p = *(np = lookup_alias(left));
        if (!*str) {
            /* delete alias */
            if (p) {
                if (echo_int) {
                    sprintf(errtext,"#deleting alias: %s=%s\r\n", left, p->subst);
					from_backend(0,errtext,strlen(errtext));
                }
                delete_aliasnode(np);
            } else {
                sprintf("#unknown alias, cannot delete: `%s'\r\n", left);
				from_backend(0,errtext,strlen(errtext));
            }
        } else {
            /* add/redefine alias */
	    
	    /* direct recursion is supported (alias CAN be defined by itself) */
            if (p) {
                free(p->subst);
                p->subst = my_strdup(right);
            } else
                add_aliasnode(left, right);
            if (echo_int) {
                sprintf(errtext,"#%s alias: %s=%s\r\n", p ? "changed" : "new",
		       left, right);
				from_backend(0,errtext,strlen(errtext));
            }
        }
    } else {
        /* show alias */
	
        *str = '\0';
        unescape(left);
	if (check_alias(left))
	    return;
        np = lookup_alias(left);
        if (*np) {
	    char buf[BUFSIZE];
            escape_specials(buf, left);
            
			sprintf(errtext, "#alias %.*s=%.*s\r\n",
		    BUFSIZE-9, buf,
		    BUFSIZE-(int)strlen(buf)-9, (*np)->subst);
			
			from_backend(0,errtext,strlen(errtext));
		
        } else {
            sprintf(errtext,"#unknown alias, cannot show: `%s'\r\n", left);
			from_backend(0,errtext,strlen(errtext));
        }
    }
}

/*
 * delete an action node
 */
static void delete_action  (actionnode ** nodep)
{
    if (echo_int) {
        sprintf(errtext,"#deleting action: >%c%s %s\r\n", (*nodep)->active ?
		   '+' : '-', (*nodep)->label, (*nodep)->pattern);
		from_backend(0,errtext,strlen(errtext));
    }
    delete_actionnode(nodep);
}

/*
 * delete a prompt node
 */
static void delete_prompt  (actionnode ** nodep)
{
    if (echo_int) {
        sprintf(errtext,"#deleting prompt: >%c%s %s\r\n", (*nodep)->active ?
		   '+' : '-', (*nodep)->label, (*nodep)->pattern);
		from_backend(0,errtext,strlen(errtext));
    }
    delete_promptnode(nodep);
}

/*
 * create new action
 */
static void add_new_action  (char * label, char * pattern, char * command, int active, int type, void * q)
{
    add_actionnode(pattern, command, label, active, type, q);
    if (echo_int) {
        sprintf(errtext,"#new action: %c%c%s %s=%s\r\n", 
		   action_chars[type],
		   active ? '+' : '-', label,
		   pattern, command);
		from_backend(0,errtext,strlen(errtext));
    }
}

/*
 * create new prompt
 */
static void add_new_prompt  (char * label, char * pattern, char * command, int active, int type, void * q)
{
    add_promptnode(pattern, command, label, active, type, q);
    if (echo_int) {
        sprintf(errtext,"#new prompt: %c%c%s %s=%s\r\n", 
		   action_chars[type],
		   active ? '+' : '-', label,
		   pattern, command);
		from_backend(0,errtext,strlen(errtext));
    }
}

/*
 * add an action with numbered label
 */
static void add_anonymous_action  (char * pattern, char * command, int type, void * q)
{
    static int last = 0;
    char label[16];
    do {
        sprintf(label, "%d", ++last);

    } while (*lookup_action(label));
    add_new_action(label, pattern, command, 1, type, q);
}

#define ONPROMPT (onprompt ? "prompt" : "action")

/*
 * change fields of an existing action node
 * pattern or commands can be NULL if no change
 */
static void change_actionorprompt  (actionnode * node, char * pattern, char * command, int type, void * q, int onprompt)
{
    if (node->type == ACTION_REGEXP && node->regexp) {
	regfree((regex_t *)(node->regexp));
	free(node->regexp);
    }
    node->regexp = q;
    if (pattern) {
        free(node->pattern);
        node->pattern = my_strdup(pattern);
	node->type = type;
    }
    if (command) {
        free(node->command);
        node->command = my_strdup(command);
    }
    
    if (echo_int) {
        sprintf(errtext,"#changed %s %c%c%s %s=%s\r\n", ONPROMPT,
		   action_chars[node->type],
		   node->active ? '+' : '-',
		   node->label, node->pattern, node->command);
		from_backend(0,errtext,strlen(errtext));
    }
}

/*
 * show defined actions
 */
void show_actions  (void)
{
    actionnode *p;
    
    sprintf(errtext,"#%s action%s defined%c\r\n", actions ? "the following" : "no",
	       (actions && !actions->next) ? " is" : "s are", actions ? ':' : '.');
	from_backend(0,errtext,strlen(errtext));
    for (p = actions; p; p = p->next) {
	sprintf(errtext,"#action %c%c%s %s=%s\r\n",
		   action_chars[p->type],
		   p->active ? '+' : '-', p->label,
		   p->pattern, p->command);
	from_backend(0,errtext,strlen(errtext));
	}
}

/*
 * show defined prompts
 */
void show_prompts  (void)
{
    promptnode *p;
    
    sprintf(errtext,"#%s prompt%s defined%c\r\n", prompts ? "the following" : "no",
	       (prompts && !prompts->next) ? " is" : "s are", prompts ? ':' : '.');
	from_backend(0,errtext,strlen(errtext));
    for (p = prompts; p; p = p->next)
	sprintf(errtext,"#prompt %c%c%s %s=%s\r\n",
		   action_chars[p->type],
		   p->active ? '+' : '-', p->label,
		   p->pattern, p->command);
	from_backend(0,errtext,strlen(errtext));
}

/*
 * parse the #action and #prompt commands
 * this function is too damn complex because of the hairy syntax. it should be
 * split up or rewritten as an fsm instead.
 */
void parse_action  (char * str, int onprompt)
{
    char *p, label[BUFSIZE], pattern[BUFSIZE], *command;
    actionnode **np = NULL;
    char sign, assign, hastail;
    char active, type = ACTION_WEAK, kind;
    void *regexp = 0;
    
    sign = *(p = skipspace(str));
    if (!sign) {
	sprintf(errtext,"%s: no arguments given\r\n", ONPROMPT);
	from_backend(0,errtext,strlen(errtext));
	return;
    }
    
    str = p + 1;
    
    switch (sign) {
      case '+':
      case '-':		/* edit */
      case '<':		/* delete */
      case '=':		/* list */
	assign = sign;
	break;
      case '%': /* action_chars[ACTION_REGEXP] */
	type = ACTION_REGEXP;
	/* falltrough */
      case '>': /* action_chars[ACTION_WEAK] */
	
	/* define/edit */
	assign = '>';
	sign = *(p + 1);
	if (!sign) {
	    sprintf(errtext,"#%s: label expected\r\n", ONPROMPT);
		from_backend(0,errtext,strlen(errtext));
	    return;
	} else if (sign == '+' || sign == '-')
	    str++;
	else
	    sign = '+';
	break;
      default:
	assign = 0;	/* anonymous action */
	str = p;
	break;
    }
    
    /* labelled action: */
    if (assign != 0) {
        p = first_regular(str, ' ');
	if ((hastail = *p))
	    *p = '\0';
	my_strncpy(label, str, BUFSIZE-1);
	if (hastail)
	    *p++ = ' ';	/* p points to start of pattern, or to \0 */
	
	if (!*label) {
	    sprintf(errtext,"#%s: label expected\r\n", ONPROMPT);
		from_backend(0,errtext,strlen(errtext));
	    return;
	}
	
	if (onprompt)
	    np = lookup_prompt(label);
	else
	    np = lookup_action(label);
	
	/* '<' : remove action */
        if (assign == '<') {
            if (!np || !*np) {
                sprintf(errtext,"#no %s, cannot delete label: `%s'\r\n",
		       ONPROMPT, label);
				from_backend(0,errtext,strlen(errtext));
            }
            else {
		if (onprompt)
		    delete_prompt(np);
		else
		    delete_action(np);
	    }
	    
	    /* '>' : define action */
        } else if (assign == '>') {

            if (sign == '+')
		active = 1;
	    else
		active = 0;
	    
	    if (!*label) {
		sprintf(errtext,"#%s: label expected\r\n", ONPROMPT);
		from_backend(0,errtext,strlen(errtext));
		return;
	    }
	    
            p = skipspace(p);
            if (*p == '(') {
		ptr pbuf = (ptr)0;
		p++;
		kind = evalp(&pbuf, &p);
		if (!REAL_ERROR && kind != TYPE_TXT)
		    error=NO_STRING_ERROR;
		if (REAL_ERROR) {
		    sprintf(errtext,"#%s: ", ONPROMPT);
			from_backend(0,errtext,strlen(errtext));
		    print_error(error=NO_STRING_ERROR);
		    ptrdel(pbuf);
		    return;
		}
		if (pbuf) {
		    my_strncpy(pattern, ptrdata(pbuf), BUFSIZE-1);
		    ptrdel(pbuf);
		} else
		    pattern[0] = '\0';
		if (*p)
		    p = skipspace(++p);
		if ((hastail = *p == '='))
		    p++;
	    }
            else {
		p = first_regular(command = p, '=');
		if ((hastail = *p))
		    *p = '\0';
		my_strncpy(pattern, command, BUFSIZE-1);
		
		if (hastail)
		    *p++ = '=';
	    }
	    
	    if (!*pattern) {
		sprintf(errtext,"#error: pattern of #%ss must be non-empty.\r\n", ONPROMPT);
		from_backend(0,errtext,strlen(errtext));
		return;
	    }
	    
	    if (type == ACTION_REGEXP && hastail) {
		int errcode;
		char unesc_pat[BUFSIZE];
		
		/*
		 * HACK WARNING:
		 * we unescape regexp patterns now, instead of doing
		 * jit+unescape at runtime, as for weak actions.
		 */
		strcpy(unesc_pat, pattern);
		unescape(unesc_pat);
		
		regexp = malloc(sizeof(regex_t)); 
		if (!regexp) {
		    errmsg("malloc");
		    return;
		}
		
		if ((errcode = regcomp((regex_t *)regexp, unesc_pat, REG_EXTENDED))) {
		    int n;
		    char *tmp;
		    n = regerror(errcode, (regex_t *)regexp, NULL, 0);
		    tmp = (char *)malloc(n);
		    if (tmp) {
			if (!regerror(errcode, (regex_t *)regexp, tmp, n))
			    errmsg("regerror");
			else {
			    sprintf(errtext,"#regexp error: %s\r\n", tmp);
				from_backend(0,errtext,strlen(errtext));
			}
			free(tmp);
		    } else {
			error = NO_MEM_ERROR;
			errmsg("malloc");
		    }
		    regfree((regex_t *)regexp);
		    free(regexp);
		    return;
		}
	    }
            command = p;
	    
            if (hastail) {
                if (np && *np) {
		    change_actionorprompt(*np, pattern, command, type, regexp, onprompt);
                    (*np)->active = active;
                } else {
		    if (onprompt)
			add_new_prompt(label, pattern, command, active, 
				       type, regexp);
		    else
			add_new_action(label, pattern, command, active, 
				       type, regexp);
		}
            }
	    
	    /* '=': list action */
        } else if (assign == '='){
            if (np && *np) {
		int len = (int)strlen((*np)->label);
		sprintf(inserted_next, "#%s %c%c%.*s %.*s=%.*s", ONPROMPT,
			action_chars[(*np)->type], (*np)->active ? '+' : '-',
			BUFSIZE - 6 /*strlen(ONPROMPT)*/ - 7, (*np)->label,
			BUFSIZE - 6 /*strlen(ONPROMPT)*/ - 7 - len, (*np)->pattern,
			BUFSIZE - 6 /*strlen(ONPROMPT)*/ - 7 - len - (int)strlen((*np)->pattern),
			(*np)->command);
            } else {
                sprintf(errtext,"#no %s, cannot list label: `%s'\r\n", ONPROMPT, label);
				from_backend(0,errtext,strlen(errtext));
            }
	    
	    /* '+', '-': turn action on/off */
        } else {
            if (np && *np) {
                (*np)->active = (sign == '+');
                if (echo_int) {
                    sprintf(errtext,"#%s %c%s %s is now o%s.\r\n", ONPROMPT,
			   action_chars[(*np)->type],
			   label,
			   (*np)->pattern, (sign == '+') ? "n" : "ff");
					from_backend(0,errtext,strlen(errtext));
                }
            } else {
                sprintf(errtext,"#no %s, cannot turn o%s label: `%s'\r\n", ONPROMPT,
		       (sign == '+') ? "n" : "ff", label);
				from_backend(0,errtext,strlen(errtext));
            }
        }
	
	/* anonymous action, cannot be regexp */
    } else {
	
	if (onprompt) {
	    tty_printf("#anonymous prompts not supported.#please use labelled prompts.\r\n");
	    return;
	}
	
        command = first_regular(str, '=');
	
        if (*command == '=') {
            *command = '\0';
	    my_strncpy(pattern, str, BUFSIZE-1);
            *command++ = '=';
            np = lookup_action_pattern(pattern);
            if (*command)
		if (np && *np)
		change_actionorprompt(*np, NULL, command, ACTION_WEAK, NULL, 0);
	    else
		add_anonymous_action(pattern, command, ACTION_WEAK, NULL);
            else if (np && *np)
		delete_action(np);
            else {
                sprintf(errtext,"#no action, cannot delete pattern: `%s'\r\n",
		       pattern);
				from_backend(0,errtext,strlen(errtext));
                return;
            }
        } else {
            np = lookup_action_pattern(str);
            if (np && *np) {
                sprintf(errtext,inserted_next, "#action %.*s=%.*s",
			BUFSIZE - 10, (*np)->pattern,
			BUFSIZE - (int)strlen((*np)->pattern) - 10,
			(*np)->command);
				from_backend(0,errtext,strlen(errtext));
            } else {
                sprintf(errtext,"#no action, cannot show pattern: `%s'\r\n", pattern);
				from_backend(0,errtext,strlen(errtext));
            }
        }
    }
}

#undef ONPROMPT

/*
 * display attribute syntax
 */
void show_attr_syntax  (void)
{
    int i;
    tty_printf("#attribute syntax:\r\n\tOne or more of:\tbold, blink, underline, inverse\r\n\tand/or\t[foreground] [ON background]\r\n\tColors: ");
    for (i = 0; i < COLORS; i++)
	sprintf(errtext,"%s%s", colornames[i],
		 (i == LOWCOLORS - 1 || i == COLORS - 1) ? "\r\n\t\t" : ",");
	from_backend(0,errtext,strlen(errtext));
    sprintf(errtext,"%s\r\n", colornames[i]);
	from_backend(0,errtext,strlen(errtext));
}

/*
 * put escape sequences to turn on/off an attribute in given buffers
 */
void attr_string  (int attrcode, char * begin, char * end)
{
    int fg = FOREGROUND(attrcode), bg = BACKGROUND(attrcode),
      tok = ATTR(attrcode);
    char need_end = 0;
    *begin = *end = '\0';

    if (tok > (ATTR_BOLD | ATTR_BLINK | ATTR_UNDER | ATTR_REVERSE)
	|| fg > COLORS || bg > COLORS || attrcode == NOATTRCODE)
      return;    /* do nothing */

    if (fg < COLORS)
	  if (bg < COLORS) 
	    sprintf(begin, "\033[3%d;4%dm", fg,bg);
	  else 
	    sprintf(begin, "\033[3%dm", fg);
    else
	  if (bg < COLORS)
		  sprintf(begin, "\033[%s%dm", bg<LOWCOLORS ? "4" : "10", bg % LOWCOLORS);
    
	switch (tok) {
		case 256 : /* bold */
			strcat(begin,"\033[1m");
			break;
		case 512 : /* underline */
			strcat(begin,"\033[4m");
			break;
		case 1024: /* inverse */
			strcat(begin,"\033[7m");
			break;
		case 2048: /* blink */
			strcat(begin,"\033[5m");
			break;
	}
}

/*
 * parse attribute description in line.
 * Return attribute if successful, -1 otherwise.
 */
int parse_attributes  (char * line)
{
    char *p;
    int tok = 0, fg, bg, t = -1;
    
    if (!(p = strtok(line, " ")))
      return NOATTRCODE;
    
    fg = bg = NO_COLOR;
    
    while (t && p) {
	if (!_strnicmp(p, "bold",4))
	  t = ATTR_BOLD;
	else if (!_strnicmp(p, "blink",5))
	  t = ATTR_BLINK;
	else if (!_strnicmp(p, "underline",9))
	  t = ATTR_UNDER;
	else if (!_strnicmp(p, "inverse",6) || !_strnicmp(p, "reverse",7))
	  t = ATTR_REVERSE;
	else
	  t = 0;
	
	if (t) {	 
	    tok |= t;
	    p = strtok(NULL, " ");
	}
    }
    
    if (!p)
	return ATTRCODE(tok, fg, bg);
    
    for (t = 0; t <= COLORS && strcmp(p, colornames[t]); t++)
	;
    if (t <= COLORS) {
	fg = t;
	p = strtok(NULL, " ");
    }
    
    if (!p)
      return ATTRCODE(tok, fg, bg);
    
    if (_strnicmp(p, "on",2))
      return -1;      /* invalid attribute */
    
    if (!(p = strtok(NULL, " ")))
      return -1;
    
    for (t = 0; t <= COLORS && strcmp(p, colornames[t]); t++)
      ;
    if (t <= COLORS)
      bg = t;
    else
      return -1;
    
    return ATTRCODE(tok, fg, bg);
}


/*
 * return a static pointer to name of given attribute code
 */
char *attr_name  (int attrcode)
{
    static char name[BUFSIZE];
    int fg = FOREGROUND(attrcode), bg = BACKGROUND(attrcode), tok = ATTR(attrcode);
    
    name[0] = 0;
    if (tok > (ATTR_BOLD | ATTR_BLINK | ATTR_UNDER | ATTR_REVERSE) || fg > COLORS || bg > COLORS)
      return name;   /* error! */
    
    if (tok & ATTR_BOLD)
      strcat(name, "bold ");
    if (tok & ATTR_BLINK)
      strcat(name, "blink ");
    if (tok & ATTR_UNDER)
      strcat(name, "underline ");
    if (tok & ATTR_REVERSE)
      strcat(name, "inverse ");
    
    if (fg < COLORS || (fg == bg && fg == COLORS && !tok))
      strcat(name, colornames[fg]);
    
    if (bg < COLORS) {
	strcat(name, " on ");
	strcat(name, colornames[bg]);
    }
    
    if (!*name)
	strcpy(name, "none");
    
    return name;
}

/*
 * show defined marks
 */
void show_marks  (void)
{
    marknode *p;
    sprintf(errtext,"#%s marker%s defined%c\r\n", markers ? "the following" : "no",
	       (markers && !markers->next) ? " is" : "s are",
	       markers ? ':' : '.');
	from_backend(0,errtext,strlen(errtext));
    for (p = markers; p; p = p->next) {
	sprintf(errtext,"#mark %s%s=%s\r\n", p->mbeg ? "^" : "",
		   p->pattern, attr_name(p->attrcode));
		from_backend(0,errtext,strlen(errtext));
	}

}


/*
 * parse arguments to the #mark command
 */
void parse_mark  (char * str)
{
    char *p;
    marknode **np, *n;
    char mbeg = 0;

    if (*str == '=') {
	tty_printf("#marker must be non-null.\r\n");
	return;
    }
    p = first_regular(str, '=');
    if (!*p) {
	if (*str ==  '^')
	    mbeg = 1, str++;
        unescape(str);
        np = lookup_marker(str, mbeg);
        if ((n = *np)) {
	    ptr pbuf = (ptr)0;
	    char *name;
	    pbuf = ptrmescape(pbuf, n->pattern, strlen(n->pattern), 0);
	    if (MEM_ERROR) { ptrdel(pbuf); return; }
            name = attr_name(n->attrcode);
            sprintf(inserted_next, "#mark %s%.*s=%.*s", n->mbeg ? "^" : "",
		    BUFSIZE-(int)strlen(name)-9, pbuf ? ptrdata(pbuf) : "",
		    BUFSIZE-9, name);
	    ptrdel(pbuf);
        } else {
            sprintf(errtext,"#unknown marker, cannot show: `%s'\r\n", str);
			from_backend(0,errtext,strlen(errtext));
        }
	
    } else {
        int attrcode, wild = 0;
	char pattern[BUFSIZE], *p2;

        *(p++) = '\0';
	p = skipspace(p);
	if (*str ==  '^')
	    mbeg = 1, str++;
	my_strncpy(pattern, str, BUFSIZE-1);
        unescape(pattern);
	p2 = pattern;
	while (*p2) {
	    if (ISMARKWILDCARD(*p2)) {
		wild = 1;
		if (ISMARKWILDCARD(*(p2 + 1))) {
		    error=SYNTAX_ERROR;
		    tty_printf("#error: two wildcards (& or $) may not be next to eachother\r\n");
		    return;
		}
	    }
	    p2++;
	}

        np = lookup_marker(pattern, mbeg);
        attrcode = parse_attributes(p);
        if (attrcode == -1) {
            tty_printf("#invalid attribute syntax.\r\n");
	    error=SYNTAX_ERROR;
            if (echo_int) show_attr_syntax();
        } else if (!*p)
	    if ((n = *np)) {
		if (echo_int) {
		    sprintf(errtext,"#deleting mark: %s%s=%s\r\n", n->mbeg ? "^" : "",
			   n->pattern, attr_name(n->attrcode));
			from_backend(0,errtext,strlen(errtext));	
		}
		delete_marknode(np);
	    } else {
		sprintf(errtext,"#unknown marker, cannot delete: `%s%s'\r\n",
		       mbeg ? "^" : "", pattern);
		from_backend(0,errtext,strlen(errtext));
	    }
        else {
            if (*np) {
                (*np)->attrcode = attrcode;
                if (echo_int) {
                    tty_printf("#changed");
                }
            } else {
                add_marknode(pattern, attrcode, mbeg, wild);
                if (echo_int) {
                    tty_printf("#new");
                }
            }
            if (echo_int)
		sprintf(errtext," mark: %s%s=%s\r\n", mbeg ? "^" : "",
			   pattern, attr_name(attrcode));
		from_backend(0,errtext,strlen(errtext));

        }
    }
}

/*
 * turn ASCII description of a sequence
 * into raw escape sequence
 * return pointer to end of ASCII description
 */
static char *unescape_seq  (char * buf, char * seq, int * seqlen)
{
    char c, *start = buf;
    enum { NORM, ESCSINGLE, ESCAPE, CARET, DONE } state = NORM;
    
    for (; (c = *seq); seq++) {
	switch (state) {
	  case NORM:
	    if (c == '^')
		state = CARET;
	    else if (c == ESC)
		state = ESCSINGLE;
	    else if (c == '=')
		state = DONE;
	    else
		*(buf++) = c;
	    break;
	  case CARET:
	    /*
	     * handle ^@ ^A  ... ^_ as expected:
	     * ^@ == 0x00, ^A == 0x01, ... , ^_ == 0x1f
	     */
	    if (c > 0x40 && c < 0x60)
		*(buf++) = c & 0x1f;
	    /* special case: ^? == 0x7f */
	    else if (c == '?')
		*(buf++) = 0x7f;
	    state = NORM;
	    break;
	  case ESCSINGLE:
	  case ESCAPE:
	    /*
	     * GH: \012 ==> octal number
	     */
	    if (state == ESCSINGLE &&
		ISODIGIT(seq[0]) && ISODIGIT(seq[1]) && ISODIGIT(seq[2])) {
		*(buf++) =(((seq[0] - '0') << 6) | 
			   ((seq[1] - '0') << 3) |
			   (seq[2] - '0'));
		seq += 2;
	    } else {
		*(buf++) = c;
		if (c == ESC)
		    state = ESCAPE;
		else
		    state = NORM;
	    }
	    break;
	  default:
	    break;
	}
	if (state == DONE)
	    break;
    }
    *buf = '\0';
    *seqlen = buf - start;
    return seq;
}

/*
 * print an escape sequence in human-readably form.
 */
void print_seq  (char * seq, int len)
{
    char ch;

    while (len--) {
	ch = *(seq++);
        if (ch == '\033') {
            tty_printf("esc ");
            continue;
        }
        if (ch < ' ') {
            tty_putc('^');
            ch |= '@';
        }
        if (ch == ' ')
	    tty_printf("space ");
        else if (ch == 0x7f)
	    tty_printf("del ");
		else if (ch & 0x80) {
	    sprintf(errtext,"\\%03o ", ch);
		from_backend(0,errtext,strlen(errtext));
		}else {
	    sprintf(errtext,"%c ", ch);
		from_backend(0,errtext,strlen(errtext));
		}
    }
}

/*
 * return a static pointer to escape sequence made printable, for use in
 * definition-files
 */
char *seq_name  (char * seq, int len)
{
    static char buf[CAPLEN*4];
    char *p = buf;
    char c;
    /*
     * rules: control chars are written as ^X, where
     * X is char | 64
     * 
     * GH: codes > 0x80  ==>  octal \012
     *
     * special case: 0x7f is written ^?
     */
    while (len--) {
	c = *seq++;
        if (c == '^' || (c && strchr(SPECIAL_CHARS, c)))
	    *(p++) = ESC;
	
	if (c < ' ') {
            *(p++) = '^';
            *(p++) = c | '@';
	} else if (c == 0x7f) {
	    *(p++) = '^';
	    *(p++) = '?';
        } else if (c & 0x80) {
	    /* GH: save chars with high bit set in octal */
	    sprintf(p, "\\%03o", (int)c);
	    p += strlen(p);
	} else
	    *(p++) = c;
    }
    *p = '\0';
    return buf;
}


static void show_single_bind  (char * msg, keynode * p)
{
    if (p->funct == key_run_command) {
	sprintf(errtext,"#%s %s %s=%s\r\n", msg, p->name,
		seq_name(p->sequence, p->seqlen),p->call_data);
	from_backend(0,errtext,strlen(errtext));
    } else {
	sprintf(errtext,"#%s %s %s=%s%s%s\r\n", msg, p->name,
		   seq_name(p->sequence, p->seqlen),
	       internal_functions[lookup_edit_function(p->funct)].name,
	       p->call_data ? " " : "",
	       p->call_data ? p->call_data : "");
	from_backend(0,errtext,strlen(errtext));
    }
}

/*
 * list keyboard bindings
 */
void show_binds  (char edit)
{
    keynode *p;
    int count = 0;
    for (p = keydefs; p; p = p->next) {
	if (edit != (p->funct == key_run_command)) {
	    if (!count) {
		if (edit) {
		    tty_printf("#line-editing keys:\r\n");
		} else {
		    tty_printf("#user-defined keys:\r\n");
		}
	    }
	    show_single_bind("bind", p);
	    ++count;
	}
    }
    if (!count) {
        tty_printf("#no key bindings defined.\r\n");
    }
}


/*
 * interactively create a new keybinding
 */
static void define_new_key  (char * name, char * command)
{    
    
	tmpfuncname=name;
	tmpcommand=command;
    sprintf(errtext,"#please press the key `%s' : ", name);
	from_backend(0,errtext,strlen(errtext));
	prompt_status=0;
	get_one_char(1);

	return;
}

void define_new_key_done (char *seq) {

	keynode *p;
	char *arg;
	int function;

	int seqlen=strlen(seq);

	    tty_putc('\r');
		tty_putc('\n');
    if (seq[0] >= ' ' && seq[0] <= '~') {
	tty_printf("#that is not a redefinable key.\r\n");
	back->send("\n",1);

	return;
    }
    
    for (p = keydefs; p; p = p->next)
	/* GH: don't allow binding of supersets of another bind */
	if (!memcmp(p->sequence, seq, MIN2(p->seqlen, seqlen))) {
	    show_single_bind("key already bound as:", p);
	    return;
	}
    
    function = lookup_edit_name(tmpcommand, &arg);
    if (function)
	add_keynode(tmpfuncname, seq, seqlen,
		    internal_functions[function].funct,arg);
    else
	add_keynode(tmpfuncname, seq, seqlen, key_run_command, tmpcommand);
    
    if (echo_int) {
	sprintf(errtext,"#new key binding: %s %s=%s\r\n",
	       tmpfuncname, seq_name(seq, seqlen), tmpcommand);
	from_backend(0,errtext,strlen(errtext));
	back->send("\n",1);
    }

}

/*
 * parse the #bind command non-interactively.
 */
static void parse_bind_noninteractive  (char * arg)
{
    char rawseq[CAPLEN], *p, *seq, *params;
    int function, seqlen;
    keynode **kp;
    
    p = strchr(arg, ' ');
    if (!p) {
        sprintf(errtext,"#syntax error: `#bind %s'\r\n", arg);
		from_backend(0,errtext,strlen(errtext));
        return;
    }
    *(p++) = '\0';
    seq = p = skipspace(p);
    
    p = unescape_seq(rawseq, p, &seqlen);
    if (!p[0] || !p[1]) {
        sprintf(errtext,"#syntax error: `#bind %s %s'\r\n", arg, seq);
		from_backend(0,errtext,strlen(errtext));
        return;
    }
    *p++ = '\0';
    
    kp = lookup_key(arg);
    if (kp && *kp)
	delete_keynode(kp);
    
    if ((function = lookup_edit_name(p, &params)))
	add_keynode(arg, rawseq, seqlen, 
		    internal_functions[function].funct, params);
    else
	add_keynode(arg, rawseq, seqlen, key_run_command, p);
    if (echo_int) {
	sprintf(errtext,"#%s: %s %s=%s\r\n", (kp && *kp) ?
		   "redefined key" : "new key binding", arg, seq, p);
	from_backend(0,errtext,strlen(errtext));
    }
}

/*
 * parse the argument of the #bind command (interactive)
 */
void parse_bind  (char *arg)
{
    char *p, *q, *command, *params;
    char *name = arg;
    keynode **npp, *np;
    int function;
    
    p = first_valid(arg, '=');
    q = first_valid(arg, ' ');
    q = skipspace(q);
    
    if (*p && *q && p > q) {	
        parse_bind_noninteractive(arg);
	return;
    }
    
    if (*p) {
        *(p++) = '\0';
        np = *(npp = lookup_key(name));
        if (*p) {
            command = p;
            if (np) {
                if (np->funct == key_run_command)
		    free(np->call_data);
		if ((function = lookup_edit_name(command, &params))) {
		    np->call_data = my_strdup(params);
		    np->funct = internal_functions[function].funct;
		} else {
                    np->call_data = my_strdup(command);
		    np->funct = key_run_command;
		}
                if (echo_int) {
                    sprintf(errtext,"#redefined key: %s %s=%s\r\n", name,
			       command);
					from_backend(0,errtext,strlen(errtext));
                }
            } else
		define_new_key(name, command);
        } else {
            if (np) {
                if (echo_int)
		    show_single_bind("deleting key binding:", np);
		delete_keynode(npp);
            } else {
                sprintf(errtext,"#no such key: `%s'\r\n", name);
				from_backend(0,errtext,strlen(errtext));
            }
        }
    } else {
        np = *(npp = lookup_key(name));
        if (np) {
	    
	    if (np->funct == key_run_command)
		sprintf(inserted_next, "#bind %.*s %s=%.*s",
			BUFSIZE-9, name, "BILLJ",
			BUFSIZE-(int)strlen(name)-9,
			np->call_data);
	    else {
		p = internal_functions[lookup_edit_function(np->funct)].name;
		sprintf(inserted_next, "#bind %.*s %s=%s%s%.*s",
			BUFSIZE-10, name, "BILLJ", p,
			np->call_data ? " " : "",
			BUFSIZE-(int)strlen(name)-(int)strlen(p)-10,
			np->call_data ? np->call_data : "");
	    }
	} else {
            sprintf(errtext,"#no such key: `%s'\r\n", name);
			from_backend(0,errtext,strlen(errtext));
        }
    }
}


/*
 * evaluate an expression, or unescape a text.
 * set value of start and end line if <(expression...) or !(expression...)
 * if needed, use/malloc `pbuf' as buffer (on error, also free pbuf)
 * return resulting char *
 */
char *redirect  (char * arg, ptr * pbuf, char * kind, char * name, int also_num, long * start, long * end)
{
    char *tmp = skipspace(arg), k;
    int type, i;
    
    if (!pbuf) {
	print_error(error=INTERNAL_ERROR);
	return NULL;
    }
    
    k = *tmp;
    if (k == '!' || k == '<')
	arg = ++tmp;
    else
	k = 0;
    
    *start = *end = 0;
    
    if (*tmp=='(') {

	arg = tmp + 1;
	type = evalp(pbuf, &arg);
	if (!REAL_ERROR && type!=TYPE_TXT && !also_num)
	    error=NO_STRING_ERROR;
	if (REAL_ERROR) {
	    sprintf(errtext,"#%s: ", name);
		from_backend(0,errtext,strlen(errtext));
	    print_error(error);
	    ptrdel(*pbuf);
	    return NULL;
	}
	for (i=0; i<2; i++) if (*arg == CMDSEP) {
	    long buf;
	    
	    arg++;
	    if (!i && *arg == CMDSEP) {
		*start = 1;
		continue;
	    }
	    else if (i && *arg == ')') {
		*end = LONG_MAX;
		continue;
	    }
	    
	    type = evall(&buf, &arg);
	    if (!REAL_ERROR && type != TYPE_NUM)
		error=NO_NUM_VALUE_ERROR;
	    if (REAL_ERROR) {
		printf("#%s: ", name);
		print_error(error);
		ptrdel(*pbuf);
		return NULL;
	    }
	    if (i)
		*end = buf;
	    else
		*start = buf;
	}
	if (*arg != ')') {
	    sprintf(errtext,"#%s: ", name);
		from_backend(0,errtext,strlen(errtext));
	    print_error(error=MISSING_PAREN_ERROR);
	    ptrdel(*pbuf);
	    return NULL;
	}
	if (!*pbuf) {
	    /* make space to add a final \r\n */
	    *pbuf = ptrsetlen(*pbuf, 1);
	    ptrzero(*pbuf);
	    if (REAL_ERROR) {
		print_error(error);
		ptrdel(*pbuf);
		return NULL;
	    }
	}
	arg = ptrdata(*pbuf);
	if (!*start && *end)
	    *start = 1;
    } else
	unescape(arg);
    
    *kind = k;
    return arg;
}

void show_vars  (void)
{    
    varnode *v;
    int i, type;
    ptr p = (ptr)0;
    
    tty_printf("#the following variables are defined:\r\n");
    
    for (type = 0; !REAL_ERROR && type < 2; type++) {
	reverse_sortedlist((sortednode **)&sortednamed_vars[type]);
	v = sortednamed_vars[type];
	while (v) {
	    if (type == 0) {
		sprintf(errtext,"#(@%s = %ld)\r\n", v->name, v->num);
		from_backend(0,errtext,strlen(errtext));

	    } else {
		p = ptrescape(p, v->str, 0);
		if (REAL_ERROR) {
		    print_error(error);
		    break;
		}
		sprintf(errtext,"#($%s = \"%s\")\r\n", v->name,
		       p ? ptrdata(p) : "");
		from_backend(0,errtext,strlen(errtext));

	    }
	    v = v->snext;
	}
	reverse_sortedlist((sortednode **)&sortednamed_vars[type]);
    }
    for (i = -NUMVAR; !REAL_ERROR && i < NUMPARAM; i++) {
	if (*VAR[i].num)
	    sprintf(errtext,"#(@%d = %ld)\r\n", i, *VAR[i].num);
		from_backend(0,errtext,strlen(errtext));

    }
    for (i = -NUMVAR; !REAL_ERROR && i < NUMPARAM; i++) {
	if (*VAR[i].str && ptrlen(*VAR[i].str)) {
	    p = ptrescape(p, *VAR[i].str, 0);
	    if (p && ptrlen(p))
		sprintf(errtext,"#($%d = \"%s\")\r\n", i, ptrdata(p));
		from_backend(0,errtext,strlen(errtext));

	}
    }
    ptrdel(p);
}

void show_delaynode  (delaynode * p, int in_or_at)
{
    long d;
    struct tm *s;
    char buf[BUFSIZE];
	
    update_now();
    d = diff_vtime(p->when, &now);
    s = localtime(p->when); // /CLOCKS_PER_SEC)); 
    /* s now points to a calendar struct */
    if (in_or_at) {
	
	if (in_or_at == 2) {
	    /* write time in buf */
	    (void)strftime(buf, BUFSIZE - 1, "%H%M%S", s);
	    sprintf(inserted_next, "#at %.*s (%s) %.*s\r\n",
		    BUFSIZE - 15, p->name, buf,
		    BUFSIZE - 15 - (int)strlen(p->name), p->command);
	}
	else
	    sprintf(inserted_next, "#in %.*s (%ld) %.*s\r\n",
		    BUFSIZE - LONGLEN - 9, p->name, d,
		    BUFSIZE - LONGLEN - 9 - (int)strlen(p->name), p->command);
    } else {
	(void)strftime(buf, BUFSIZE - 1, "%H:%M:%S", s);
	sprintf(errtext,"#at (%s) #in (%ld) `%s' %s\r\n", buf, d, p->name, p->command);
	from_backend(0,errtext,strlen(errtext));
    }
}

void show_delays  (void)
{
    delaynode *p;
    int n = (delays ? delays->next ? 2 : 1 : 0) +
	(dead_delays ? dead_delays->next ? 2 : 1 : 0);
    
    sprintf(errtext,"#%s delay label%s defined%c\r\n", n ? "the following" : "no",
	       n == 1 ? " is" : "s are", n ? ':' : '.');
	from_backend(0,errtext,strlen(errtext));

    for (p = delays; p; p = p->next)
	show_delaynode(p, 0);
    for (p = dead_delays; p; p = p->next)
	show_delaynode(p, 0);
}

void change_delaynode  (delaynode ** p, char * command, long millisec)
{
    delaynode *m=*p;
    
    *p = m->next;
    *m->when = millisec;
    
    update_now();
    add_vtime(m->when, &now);
    if (*command) {
	if (strlen(command) > strlen(m->command)) {
	    free((void*)m->command);
	    m->command = my_strdup(command);
	}
	else
	    strcpy(m->command, command);
    }
    if (millisec < 0)
	add_node((defnode*)m, (defnode**)&dead_delays, rev_time_sort);
    else
	add_node((defnode*)m, (defnode**)&delays, time_sort);
    if (echo_int) {
	printf("#changed ");
	show_delaynode(m, 0);
    }
}

void new_delaynode  (char * name, char * command, long millisec)
{
    time_t t;
    delaynode *node;
    
    t= millisec;
	//t.tv_usec = (millisec % mSEC_PER_SEC) * uSEC_PER_mSEC;
    //t.tv_sec  =  millisec / mSEC_PER_SEC;
    update_now();
    add_vtime(&t, &now);
    node = add_delaynode(name, command, &t, millisec < 0);
    if (echo_int && node) {
	tty_printf("#new ");
	show_delaynode(node, 0);
    }
}

void show_history  (int count)
{
    int i = curline;
    
    if (!count) count = lines - 1;
    if (count >= MAX_HIST) count = MAX_HIST - 1;
    i -= count;
    if (i < 0) i += MAX_HIST;
    
    while (count) {
	if (hist[i]) {
		sprintf(errtext,"#%2d: %s\r\n", count, hist[i]);
	//	update_term_buf(errtext,strlen(errtext));
		from_backend(0,errtext,strlen(errtext));

	}
	count--;
	if (++i == MAX_HIST) i = 0;
    }
	tty_printf("\r\n");
}

void exe_history  (int count)
{
    int i = curline;
    char buf[BUFSIZE];
    
    if (count >= MAX_HIST)
	count = MAX_HIST - 1;
    i -= count;
    if (i < 0)
	i += MAX_HIST;
    if (hist[i]) {
	strcpy(buf, hist[i]);
	parse_user_input(buf, 0);
    }
}


void parse_rebind (char *arg)
{
    char  *seq;
    keynode **kp;
    
    arg = skipspace(arg);
    if (!*arg) {
	tty_printf("#rebind: missing key.\n");
	return;
    }
    
    seq = first_valid(arg, ' ');
    if (*seq) {
	*seq++ = '\0';
	seq = skipspace(seq);
    }
    
    kp = lookup_key(arg);
    if (!kp || !*kp) {
	sprintf(errtext,"#no such key: `%s'\r\n", arg);
	from_backend(0,errtext,strlen(errtext));
	return;
    }
	// this temp variable is to hold the keynode that we're looking at
	// until the new key is pressed 

	tmpkeynode=*kp;

	sprintf(errtext,"#please press the key `%s' : ", arg);
	from_backend(0,errtext,strlen(errtext));


	get_one_char(2);

}   
    


void parse_rebind_done(char *seq) {
	keynode *p;
	char **old;
    int seqlen;

	seqlen=strlen(seq);

	// now, we've saved the keynode before while waiting for user input

	 for (p = keydefs; p; p = p->next) {
		if (p == tmpkeynode)
			continue;
		if (seqlen == p->seqlen && !memcmp(p->sequence, seq, seqlen)) {
		    show_single_bind("key already bound as:", p);
		    return;
		}
	}
    
	old = &(tmpkeynode->sequence);
	if (*old)		
	free(*old);
	
	*old = (char *)malloc(tmpkeynode->seqlen = seqlen);
    memmove(*old, seq, seqlen);

	show_single_bind("redefined key:", tmpkeynode);
}


