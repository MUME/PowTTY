#include <windows.h>
#include <imm.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "powdefines.h"
#include "powmain.h"
#include "powhook.h"
#include "powcmd2.h"
#include "putty.h"
#include "powlog.h"
#include "powedit.h"

void tty_printf(char *args) {
	from_backend(0,args,strlen(args));
	return;


}

void tty_putc(char c) {
	from_backend(0,&c,1);
	
	return;

}



void wrap_print(char *arg) {
		from_backend(0,arg,strlen(arg));

}

void tty_puts(char *arg) {

	char *tmp;
	MessageBox(NULL,arg,"tty_puts",MB_OK);
	tmp=malloc(strlen(arg));
	strncpy(tmp,arg,strlen(arg));
		from_backend(0,tmp,strlen(tmp));
		free(tmp);

}

void errmsg (char *arg) {

		from_backend(0,arg,strlen(arg));

}



void input_overtype_follow (char c) {
	add_term_buf(&c,1);

}


void input_insert_follow_chars(char *p,int n) {
	add_term_buf(p,n);

}

void get_one_char(int x) {
	// 1 is for a new bind
	// 2 is for a rebind

	char_wait_flag=x;
	

}

void get_one_char_done(char *c) {

	
	if (char_wait_flag==1) {
	define_new_key_done(c);
	} else if(char_wait_flag==2) {
	parse_rebind_done(c);
	}

	char_wait_flag=0;

}


void powGetLastError() {
	LPVOID lpMessageBuffer;

	FormatMessage(
  FORMAT_MESSAGE_ALLOCATE_BUFFER |
  FORMAT_MESSAGE_FROM_SYSTEM,
  NULL,
  GetLastError(),
  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
  (LPTSTR) &lpMessageBuffer,
  0,
  NULL );

	MessageBox(NULL,lpMessageBuffer,"PowTTY Oops!",MB_OK);

// Free the buffer allocated by the system

	LocalFree( lpMessageBuffer );

}

