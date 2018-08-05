/*
 *  tty.c -- terminal handling routines for powwow
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
#include <time.h>
#include <sys/types.h>

#include "powdefines.h"
#include "powmain.h"
#include "powedit.h"
#include "powutils.h"
#include "powlist.h"
#include "powtty.h"
#include "powhook.h"

#define insertfinish (0)
static int len_begoln = 1, len_leftcur = 3, len_upcur = 3, gotocost = 8;



int tty_read_fd = 0;
static int wrapglitch = 0;


/*
 * Terminal handling routines:
 * These are one big mess of left-justified chicken scratches.
 * It should be handled more cleanly...but unix portability is what it is.
 */

/*
 * Set the terminal to character-at-a-time-without-echo mode, and save the
 * original state in ttybsave
 */
void tty_start  (void)
{
    

}

/*
 * Reset the terminal to its original state
 */
void tty_quit  (void)
{

}

/*
 * enable/disable special keys depending on the current linemode
 */
void tty_special_keys  (void)
{

}


/*
 * add the default keypad bindings to the list
 */
void tty_add_walk_binds  (void)
{
    /*
     * Note: termcap doesn't have sequences for the numeric keypad, so we just
     * assume they are the same as for a vt100. They can be redefined
     * at runtime anyway (using #bind or #rebind)
     */
	add_keynode("KP0", "\033Op", 3, key_run_command, "flee");
    add_keynode("KP2", "\033Or", 3, key_run_command, "s");
    add_keynode("KP3", "\033Os", 3, key_run_command, "d");
    add_keynode("KP4", "\033Ot", 3, key_run_command, "w");
    add_keynode("KP5", "\033Ou", 3, key_run_command, "exits");
    add_keynode("KP6", "\033Ov", 3, key_run_command, "e");
    add_keynode("KP7", "\033Ow", 3, key_run_command, "look");
    add_keynode("KP8", "\033Ox", 3, key_run_command, "n");
    add_keynode("KP9", "\033Oy", 3, key_run_command, "u");
    }

/*
 * initialize the key binding list
 */
void tty_add_initial_binds  (void)
{
    struct b_init_node {
	char *label, *seq;
	function_str funct;
    };
    static struct b_init_node b_init[] = {
	{ "Tab",	"\t",		complete_word },
	{ "C-n",	"\016",		next_line },
	{ "C-p",	"\020",		prev_line },
	{ "",		"",		0 }
    };
    struct b_init_node *p = b_init;

	
    do {
	add_keynode(p->label, p->seq, 1, p->funct, NULL);
    } while((++p)->seq[0]);

	add_keynode("Down" , "\x1B[B", 3, next_line, NULL);
    add_keynode("Up"   , "\x1B[A" ,3, prev_line, NULL);

}




