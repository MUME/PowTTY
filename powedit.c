/*
 *  edit.c  --  line editing functions for powwow
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>


#include "powdefines.h"
#include "powmain.h"
#include "powutils.h"
#include "powtty.h"
#include "powcmd.h"
#include "powedit.h"
#include "poweval.h"
#include "powlog.h"
#include "powtty.h"
#include "powhook.h"
#include "putty.h"

/* history buffer */
char *hist[MAX_HIST];	/* saved history lines */
int curline = 0;	/* current history line */
int pickline = 0;	/* line to pick history from */

/* word completion list */
wordnode words[MAX_WORDS];
int wordindex = 0;



edit_function internal_functions[] = {
    {(char *)0, (function_str)0, },
    {"&enter-line", enter_line, },
    {"&complete-word", complete_word, },
    {"&complete-line", complete_line, },
    {"&prev-line", prev_line, },
    {"&next-line", next_line, },
    {"&to-history", to_history, },
    {"&clear-line", clear_line, },

    {(char *)0, (function_str)0 }

};

int lookup_edit_name (char *name, char **arg)
{
    int i, len, flen;
    char *fname, *extra = NULL;
    
    if ((fname = strchr(name, ' ')))
	len = fname - name;
    else
	len = strlen(name);
    
    for (i=1; (fname = internal_functions[i].name); i++) {
	flen = strlen(fname);
	if (flen == len && !strncmp(name, fname, flen)) {
	    extra = name + flen;
	    if (*extra == ' ') extra++;
	    if (!*extra) extra = NULL;
	    *arg = extra;
	    return i;
	}
    }
    *arg = extra;
    return 0;
}

int lookup_edit_function (function_str funct)
{
    int i;
    function_str ffunct;
    
    for (i = 1; (ffunct = internal_functions[i].funct); i++)
	if (funct == ffunct)
	    return i;
    
    return 0;
}


/*
 * move a line into history, but don't do anything else
 */
void to_history  (char *edbuf)
{
    put_history(edbuf);
    pickline = curline;
   // *edbuf = '\0';
    // pos = edlen = 0;
}

/*
 * put string in history at current position
 * (string is assumed to be trashable)
 */
void put_history  (char *str)
{

    char *p;
	int len=strlen(str);
	int i,j;
	// don't put one line commands into the history

	if (len==1) {
		return;
	} 

	for (i=0;i<len;i++) {
		if ((str[i]=='\\') && (str[i+1]=='\\')) {
			for (j=i;j<len;j++) {
				str[j]=str[j+1];
			}
		}

	}

	if (curline<0) curline=0;
	if(curline>0) {
		if (strcmp(hist[curline-1],str) == 0) {
			return;	
		}
	}

    if (hist[curline]) free(hist[curline]);
    if (!(hist[curline] = my_strdup(str))) {
	errmsg("malloc");
	return;
    }
    if (++curline == MAX_HIST)
	curline = 0;

    /* split into words and put into completion list */
    for (p = strtok(str, DELIM); p; 
	 p = strtok(NULL, DELIM)) {
        if (strlen(p) >= MIN_WORDLEN && 
	    p[0] != '#') /* no commands/short words */
	    put_word(p);
    }
}

/*
 * move a node before wordindex, i.e. make it the last word
 */
static void demote_word  (int i)
{
    words[words[i].prev].next = words[i].next;
    words[words[i].next].prev = words[i].prev;
    words[i].prev = words[words[i].next = wordindex].prev;
    words[wordindex].prev = words[words[wordindex].prev].next = i;
}

/*
 * match and complete a word referring to the word list
 */
void complete_word  (char *dummy)
{
    /*
     * GH: rewritten to allow circulating through history with repetitive command
     *     code stolen from cancan 2.6.3a
     *        curr_word:   index into words[]
     *        comp_len     length of current completition
     *        root_len     length of the root word (before the completition)
     *        root         start of the root word
     */	
    
    static int curr_word, comp_len = 0, root_len = 0;
    char *root, *p;
    int k, n;
    /* find word start */
    if (last_edit_cmd == (function_str)complete_word && comp_len) {
	k = comp_len;
	input_delete_nofollow_chars(k);
	n = pos - root_len;
    } else {
	for (n = pos; n > 0 && !IS_DELIM(edbuf[n - 1]); n--)
	    ;
	k = 0;
	curr_word = wordindex;
	pos = strlen(edbuf);
	root_len = pos - n;
    }
    root = edbuf + n; comp_len = 0;

    /* k = chars to delete,  n = position of starting word */
    
    /* scan word list for next match */

	/* 
		this while loop will *not* execute!! Bill is stuck here..
		had it working once
	*/
    while ((p = words[curr_word = words[curr_word].next].word)) {
		
		//		BILLJ hack from strncasecmp
	if (!_strnicmp(p, root, root_len) &&
	    *(p += root_len) &&
	    (n = strlen(p)) + edlen < BUFSIZE) {
	    comp_len = n;
	    for (; k && n; k--, n--)
		input_overtype_follow(*p++);

	    if (n > 0)
		input_insert_follow_chars(p, n);
	    break;
	}
    }
    
    /* delete duplicate instances of the word */
    if (p && !(words[k = curr_word].flags & WORD_UNIQUE)) {
	words[k].flags |= WORD_UNIQUE;
	p = words[k].word;  
	n = words[k].next;
	while (words[k = n].word) {
	    n = words[k].next;
	    if (!strcmp(p, words[k].word)) {
		demote_word(k);
		free(words[k].word);
		words[k].word = 0;
		words[curr_word].flags |= words[k].flags;	/* move retain flag */
		if ((words[k].flags &= WORD_UNIQUE))
		    break;
	    }
	}
    }
}

/*
 * match and complete entire lines backwards in history
 * GH: made repeated complete_line cycle through history
 */
void complete_line  (char *dummy)
{
    static int curr_line = MAX_HIST-1, root_len = 0, first_line = 0;
    int i;
    
    if (last_edit_cmd != (function_str)complete_line) {
	root_len = edlen;
	first_line = curr_line = curline;
    }
    
    for (i = curr_line - 1; i != curr_line; i--) {
	if (i < 0) i = MAX_HIST - 1;
	if (i == first_line)
	    break;
	if (hist[i] && !strncmp(edbuf, hist[i], root_len))
	    break;
    }
    if (i != curr_line) {
	clear_input_line(0);
	if (i == first_line) {
	    edbuf[root_len] = 0;
	    edlen = root_len;
	} else {
	    strcpy(edbuf, hist[i]);
	    edlen = strlen(edbuf);
	}
	pos = edlen;
	curr_line = i;
    }
}

/*
 * GH: word history handling stolen from cancan 2.6.3a
 */

static void default_completions  (void)
{
    char buf[BUFSIZE];
    cmdstruct *p;
    int i;
    for (i = 0, buf[0] = '#', p = commands; p->name; p++)
	if (p->funct /*&& strlen(p->name) >= 3*/ ) {
	    if (++i >= MAX_WORDS) break;
	    strcpy(buf + 1, p->name);
	    if (!(words[i].word = my_strdup(buf)))
	    words[i].flags = WORD_UNIQUE | WORD_RETAIN;
	}
    for (i = MAX_WORDS; i--; words[i].prev = i - 1, words[i].next = i + 1)
	;
    words[0].prev = MAX_WORDS - 1;
    words[MAX_WORDS - 1].next = 0;
}

/*
 * put word in word completion ring
 */
void put_word  (char *s)
{
    int r = wordindex;

    if (!(words[r].word = my_strdup(s))) {
	errmsg("malloc");
	return;
    }
    words[r].flags = 0;
    while (words[r = words[r].prev].flags & WORD_RETAIN)
	;
    demote_word(r);
    wordindex = r;
    if (words[r].word) { 
	free(words[r].word); 
	words[r].word = 0; 
    }
}

/*
 * GH: set delimeters[DELIM_CUSTOM]
 */
void set_custom_delimeters  (char *s)
{
    char *old = delim_list[DELIM_CUSTOM];
    if (!(delim_list[DELIM_CUSTOM] = my_strdup(s)))
	errmsg("malloc");
    else {
	if (old)
	    free(old);
	delim_len[DELIM_CUSTOM] = strlen(s);
	delim_mode = DELIM_CUSTOM;
    }
}

void clear_line  (char *dummy)
{
    if (!edlen)
	return;
    clear_input_line(0);
    pickline = curline;
    *edbuf = '\0';
    pos = edlen = 0;
}

/*
 * enter a line
 */
void enter_line  (char *dummy)
{
    char *p;
    
  history_done = 0;
    
    /* don't put identical lines in history, nor short ones */
    p = hist[curline ? curline - 1 : MAX_HIST - 1];
    if (!p || (edlen > 1 && strcmp(edbuf, p)))
	put_history(edbuf);
    pickline = curline;
	edbuf[0] = '\0';
    pos = edlen = strlen(edbuf);
}


/*
 * get previous line from history list
 */
void prev_line  (char *dummy)
{

    int i = pickline - 1;

    if (i < 0) i = MAX_HIST - 1;
    if (hist[i]) {
	if (hist[pickline] && strcmp(hist[pickline], edbuf)) {
	    free(hist[pickline]);
	    hist[pickline] = NULL;
	}
	if (!hist[pickline]) {
	    if (!(hist[pickline] = my_strdup(edbuf))) {
		errmsg("malloc");

		return;
	    }
	}

	pickline = i;
	update_term_buf(hist[pickline],strlen(hist[pickline]));
	
    }
}

/*
 * get next line from history list
 */
void next_line  (char *dummy)
{
    int i = pickline + 1;
    if (i == MAX_HIST) i = 0;
    if (hist[i]) {
	if (hist[pickline] && strcmp(hist[pickline], edbuf)) {
	    free(hist[pickline]);
	    hist[pickline] = NULL;
	}
	if (!hist[pickline]) {
	    if (!(hist[pickline] = my_strdup(edbuf))) {
		errmsg("malloc");
		return;
	    }
	}
	pickline = i;
	update_term_buf(hist[pickline],strlen(hist[pickline]));
    }
}



/*
 * execute string as if typed
 */
void key_run_command  (char *cmd)
{
    clear_input_line(opt_compact && !echo_key);
    if (echo_key) {
	sprintf(errtext,"%s%s%s\r\n", edattrbeg, cmd, edattrend);
	from_backend(0,errtext,strlen(errtext));
    } else if (!opt_compact)
        tty_printf("\r\n");
    
    error = 0;
    
    parse_instruction(cmd, 1, 0, 1);
    history_done = 0;
}

void edit_bootstrap (void)
{
    default_completions();
}


void draw_prompt (void)
{
    if (promptlen && prompt_status == 1) {
	int e = error;
	error = 0;
	marked_prompt = ptraddmarks(marked_prompt, prompt->str);
	if (MEM_ERROR) { promptzero(); errmsg("malloc(prompt)"); return; }

	from_backend(0,ptrdata(marked_prompt),strlen(ptrdata(marked_prompt)));
	//col0 = printstrlen(promptstr); /* same as printstrlen(marked_prompt) */
	error = e;
    }
    prompt_status = 0;
}


/*
 * clear current input line (deleteprompt == 1 if to clear also prompt)
 * cursor is left right after the prompt.
 * 
 * since we do not expect data from the user at this point,
 * do not print edattrbeg now.
 */



/*
 * delete word to the right
 */
void del_word_right  (char *dummy)
{
    int i;
    for (i = pos; edbuf[i] && !isalnum(edbuf[i]); i++)
	;
    while (isalnum(edbuf[i]))
	i++;
    input_delete_nofollow_chars(i - pos);
}


