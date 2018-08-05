/*
 *  beam.c  --  code to beam texts across the TCP connection following a
 *              special protocol
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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <io.h>			
#include <fcntl.h>		
#include <windows.h>
#include <process.h>  

#include "powdefines.h"
#include "powmain.h"
#include "powutils.h"
#include "powbeam.h"
#include "powmap.h"
#include "powtty.h"
#include "powedit.h"
#include "poweval.h"
#include "powhook.h"
#include "putty.h"

editsess *edit_sess;	/* head of session list */

char edit_start[BUFSIZE]; /* messages to send to host when starting */ 
char edit_end[BUFSIZE];   /* or leaving editing sessions */ 

#define IAC 255



static DWORD WINAPI watcher(LPVOID info)
    {
	watchInfo * watchStruct=(watchInfo *) info;
    WaitForSingleObject(watchStruct->process,INFINITE);
    PostMessage(watchStruct->hwnd, UWM_PROCESS_STOPPED,0,0); 
	if (watchStruct) free(watchStruct);
    return 0;
   }



static int tcp_addIAC (char *dest, char *txt, int len)
{
    char *s = dest;
    while (len-- > 0) {
	if ((*s++ = *txt++) == (char)(byte)IAC)
	    *s++ = (char)(byte)IAC;
    }
    return s - dest;
}


static int tcp_read_addIAC (int fd, char *data, int len)
{
    char *s = data;
    char buf[BUFSIZE];
    int i;
    
    while (len > 0) {
	while ((i = read(fd, buf, MIN2(len, BUFSIZE))) < 0 && errno == EINTR)
	    ;
	if (i < 0) {
	    errmsg("read from file");
	    return -1;
	} else if (i == 0)
	    break;
	s += tcp_addIAC(s, buf, i);
	len -= i;
    }
    return s - data;
}


static void write_message (char *s)
{
    clear_input_line(opt_compact);
    if (!opt_compact) {
	tty_printf("\r\n");
	status(1);
    }
	tty_printf(s);
//    tty_puts(s);
}

/*
 * abort an editing session when
 * the MUD socket it comes from gets closed
 */
void abort_edit_fd  (int fd)
{
    editsess **sp, *p;

    if (fd < 0) {
		return;
	}
    
    for (sp = &edit_sess; *sp; sp = &(*sp)->next) {
		p = *sp;
		if (p->fd != fd)
			continue;
    
		if (TerminateProcess(p->pid,0) ==0) {       /* Editicide */
		    errmsg("kill editor child");
		    continue;
		}

		printf("#aborted `%s' (%u)\n", p->descr, p->key);
		p->cancel = 1;
    }
}

/*
 * cancel an editing session; does not free anything
 * (the child death signal handler will remove the session from the list)
 */
void cancel_edit  (editsess *sp)
{
    char buf[BUFSIZE];
    char keystr[16];
    
		if (TerminateProcess(sp->pid,0) ==0) {       /* Editicide */
		    errmsg("kill editor child");
		    return;
		}

    sprintf(errtext,"#killed `%s' (%u)\n", sp->descr, sp->key);
	from_backend(0,errtext,strlen(errtext));
    sprintf(keystr, "C%u\n", sp->key);
    sprintf(buf, "%sE%d\n%s", MPI, (int)strlen(keystr), keystr);
    back->send(buf,strlen(buf));
    sp->cancel = 1;
}

/*
 * send back edited text to server, or cancel the editing session if the
 * file was not changed.
 */
static void finish_edit  (editsess *sp)
{
    char *realtext = NULL, *text;
    int fd, txtlen, hdrlen;
    struct stat sbuf;
    char keystr[16], buf[256], hdr[65];
   
    if (sp->fd == -1)
	goto cleanup_file;
    
    fd = _open(sp->file, _O_RDONLY);
    if (fd == -1) {
		Sleep(2000);
		fd = _open(sp->file, _O_RDONLY);
		if (fd==-1) {
			powGetLastError();
			errmsg("open edit file");
			goto cleanup_file;
		}
    }
    if (fstat(fd, &sbuf) == -1) {
	errmsg("fstat edit file");
	goto cleanup_fd;
    }
    
    txtlen = sbuf.st_size;

    if (!sp->cancel && (sbuf.st_mtime > sp->ctime || txtlen != sp->oldsize)) {
	/* file was changed by editor: send back result to server */
	
	realtext = (char *)malloc(2*txtlen + 65);
	/* *2 to protect IACs, +1 is for possible LF, +64 for header */
	if (!realtext) {
	    errmsg("malloc");
	    goto cleanup_fd;
	}
	
	text = realtext + 64;
	if ((txtlen = tcp_read_addIAC(fd, text, txtlen)) == -1)
	    goto cleanup_text;
	
	if (txtlen && text[txtlen - 1] != '\n') {
	    /* If the last line isn't LF-terminated, add an LF;
	     * however, empty files must remain empty */
	    text[txtlen] = '\n';
	    txtlen++;
	}
	
	sprintf(keystr, "E%u\n", sp->key);
	
	sprintf(hdr, "%sE%d\n%s", MPI, (int)(txtlen + strlen(keystr)), keystr);
	
	text -= (hdrlen = strlen(hdr));
	memcpy(text, hdr, hdrlen);
	
	text[hdrlen + txtlen] = '\0'; 
	// tcp_raw_write(text);
	back->send(text,strlen(text));

	sprintf(buf, "#completed session %s (%u)\r\n", sp->descr, sp->key);
	write_message(buf);
    } else {
	/* file wasn't changed, cancel editing session */
	sprintf(keystr, "C%u\n", sp->key);
	sprintf(hdr, "%sE%d\n%s", MPI, (int) strlen(keystr), keystr);
	
//	tcp_raw_write(hdr);
	back->send(hdr,strlen(hdr));
	
	sprintf(buf, "#cancelled session %s (%u)\n", sp->descr, sp->key);
	write_message(buf);
    }
    
cleanup_text: if (realtext) free(realtext);
cleanup_fd:   close(fd);
cleanup_file: if (_unlink(sp->file) < 0)
		errmsg("unlink edit file");
}

/*
 * start an editing session: process the EDIT/VIEW message
 * if view == 1, text will be viewed, else edited
 */
void message_edit (char *text, int msglen, char view, char builtin)
{

    char tmpname[BUFSIZE], command_str[BUFSIZE];
    char *errdesc = "#warning: protocol error (message_edit, no %s)\n";
    int tmpfd, i;
    unsigned int key;
    editsess *s;
    char *editor, *descr,*tmpdir;
    int waitforeditor;
	PROCESS_INFORMATION procInfo;

	STARTUPINFO InfoStart;
	struct stat sbuf;

	 memset(&InfoStart,0,sizeof(InfoStart));
	InfoStart.cb = sizeof(InfoStart.cb);

  
    status(1);

    if (view) {
	key = (unsigned int)-1;
	i = 0;
    } else {
		if (text[0] != 'M') {
			//tty_printf(errdesc, "M");
			sprintf(errtext,errdesc, "M");
			from_backend(0,errtext,strlen(errtext));
		//	free(text);
			return;
		
		}
	for (i = 1; i < msglen && isdigit(text[i]); i++)
	    ;
	if (text[i++] != '\n' || i >= msglen) {
	    sprintf(errtext,errdesc, "\\n");
		from_backend(0,errtext,strlen(errtext));
	    free(text);
	    return;
	}
	key = strtoul(text + 1, NULL, 10);
    }
    descr = text + i;
    while (i < msglen && text[i] != '\n') i++;
    if (i >= msglen){
	sprintf(errtext,errdesc, "desc");
	from_backend(0,errtext,strlen(errtext));
	//free(text);
	return;
    }
    text[i++] = '\0';

	// open the file
	if (!(tmpdir=cfg.powwowtmp)) {
		strcpy(tmpdir,"c:\\windows\\temp");
	}
	if (tmpdir[strlen(tmpdir)-1] != '\\') {
		sprintf(tmpdir,"%s\\",tmpdir);
	}
	
    sprintf(tmpname, "%spowwow.%u.%d%d.txt",tmpdir, key, _getpid(), abs(rand()) >> 8);

    if ((tmpfd = _open(tmpname, _O_WRONLY|_O_CREAT,_S_IREAD | _S_IWRITE)) < 0) {
		errmsg("create temp edit file");
		powGetLastError();
		free(text);
		return;
		}
    if (_write(tmpfd,text + i, msglen - i) < msglen - i) {
		errmsg("write to temp edit file");
	//	free(text);
		_close(tmpfd);
		return;
		}

	// I changed from the orig powwow code here.. I want to do an fstat
	// to determine the real file size right now.. instead of using msglen

	fstat(tmpfd,&sbuf);

	_close(tmpfd);
    
	//if (text) free(text);

    s = (editsess*)malloc(sizeof(editsess));
	if (!s) {
		errmsg("malloc");
		return;
	}

	s->ctime = sbuf.st_mtime;
	s->oldsize = sbuf.st_size;
	s->key = key;
	s->fd = (view || builtin) ? -1 : 0; /* MUME doesn't expect a reply. billj removed the tcp_fd */
	s->cancel = 0;
	s->descr = my_strdup(descr);
	s->file = my_strdup(tmpname);


		/* send a edit_start message (if wanted) */ 
    if ((!edit_sess) && (*edit_start)) {
		error = 0;
		parse_instruction(edit_start, 0, 0, 1);
		history_done = 0;
	}


    if (view) editor=cfg.powwowpager;
	else editor=cfg.powwoweditor;
		
	// never 'wait for editor' on Win32
   
	waitforeditor = 0;

    // open the file in the editor

	// strip any pesky quotes that someone may have used to specify
	// their editor -- we cannot pass them as 'editor', but need them as 'command_str'

	
	if (editor[0] =='"') {
		for (i=0;i<strlen(editor);i++) {
			editor[i]=editor[i+1];
		}
	}
	i=strlen(editor);
	if (editor[i]=='"') {
			editor[i]='\0';
	}
	
	
	
	sprintf(command_str,"\"%s\" \"%s\"",editor,tmpname);

	if (!CreateProcess(editor,command_str,NULL,NULL,0,NORMAL_PRIORITY_CLASS,NULL,tmpdir,&InfoStart,&procInfo)) {
		errmsg("could not launch viewer!");
		powGetLastError(); 
	} else {
		  DWORD junk;

          watchInfo * watchStruct = malloc(sizeof(watchInfo));
          watchStruct->hwnd =getmyhwnd();  //...window to notify on completion
          watchStruct->process = procInfo.hProcess;
			
           CreateThread(NULL,0,&watcher, (LPVOID)watchStruct,0,&junk);         

	}
		  
	s->pid=procInfo.hProcess;
    
	if (edit_sess) {
		s->next = edit_sess;
	} else {
		s->next=NULL;
	}
	edit_sess = s;
}

/*
 * Changed to be: have any children rip'd ?
 * 
 */

void sig_chld_bottomhalf  (void)
{
    editsess **sp,*p;
   
	DWORD tmp;
	int test;
	for (sp = &edit_sess; *sp; sp = &(*sp)->next) {
			test=GetExitCodeProcess((*sp)->pid,&tmp);

			if (tmp != STILL_ACTIVE) {
				
					finish_edit(*sp);
					p=*sp;
					*sp=p->next;
				//	free(p->next);
				//	free(p->descr);
					free(p);
			/* send the edit_end message if this is the last editor... */ 
	
				if ((!edit_sess) && (*edit_end)) {
				error = 0;
				parse_instruction(edit_end, 0, 0, 1);
				history_done = 0;
				}
				break;
			}
    }
	
}

