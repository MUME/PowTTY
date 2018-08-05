#include "putty.h"


#ifndef POWHOOK_H__
#define POWHOOK_H__

void errmsg(char *arg);

void tty_printf(char *args);
void tty_putc(char c);
void clear_input_line(int x);

/* powutils.c */

void wrap_print(char *arg);
void tty_puts(char *arg);


/* powedit.c */ 
void input_overtype_follow (char c);
void input_delete_nofollow_chars(int n); // delete at N, and insert
void input_insert_follow_chars(char *p,int n); // insert at N 

/* pwcmd2.c */
void get_one_char(int x);
void get_one_char_done(char *c);

/* by all of pow* */
char errtext[512];
int char_wait_flag;
typedef struct {HWND hwnd; HANDLE process; } watchInfo;


/* by both apps */
void process_telnet_text(char *data,int len);
void powGetLastError();
void processed_term_out(int done);
char last_printed;
char current_session[255];
#endif

