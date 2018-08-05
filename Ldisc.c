#include <windows.h>
#include <stdio.h>
#include <ctype.h>

#include "putty.h"
#include "powhook.h"
#include "powdefines.h"
#include "powmain.h"
#include "powedit.h"

static int has_cleared; // this is for the input line


/*
 * ldisc.c: PuTTY line disciplines
 */


static char *term_buf = NULL;
static int term_buflen = 0, term_bufsiz = 0, term_quotenext = 0;
static int insert_mode=1; // are we in insert_mode? the insert key toggles

term_curpos=0;

static void c_write (char *buf, int len) {
	from_backend(0, buf, len);

}

static void c_write1 (char c) {
    c_write(&c, 1);
}



static int plen(unsigned char c) {
    if ((c >= 32 && c <= 126) ||
        (c >= 160))
        return 1;
    else if (c < 128)
        return 2;                      /* ^x for some x */
    else
        return 4;                      /* <XY> for hex XY */
}

static void pwrite(unsigned char c) {
    if ((c >= 32 && c <= 126) ||
        (c >= 160)) {
        c_write1(c);
    } else if (c < 128) {
        char cc[2];
        cc[1] = (c == 127 ? '?' : c + 0x40);
		cc[0] = '^';
		c_write(cc, 2);
    } else {
        char cc[5];
        sprintf(cc, "<%02X>", c);
        c_write(cc, 4);
    }
	has_cleared=0;
}

static void bsb(int n) {
    while (n--)
	c_write("\010 \010", 3);
}

/* added by billj */ 
static void termbuf_delete_at(int pos) {
	int i;

	if (term_buflen==0) return;

	/* pos is negative */

	
	for (i=pos;i<0;i++) {
		if (term_buf[term_buflen+i+1] != '\0') {
			term_buf[term_buflen+i]=term_buf[term_buflen+i+1];
		} else {
			term_buf[term_buflen+i]=' ';			
		}
		pwrite(term_buf[term_buflen+i]);
	}
	

	inbuf_head--;
	inbuf[inbuf_head]='\0';

	term_buflen--;

	return;
}


#define CTRL(x) (x^'@')

static void term_send(char *buf, int len) {
	int i=0;

		if (char_wait_flag) {
		get_one_char_done(buf);
		return;

	}


	if (buf[0]=='\x1B') {
		int done=0;

		switch (buf[1]) {
		case '[':
			// if this ends up being a '14' for F4, for example,
			// then just let powwow deal, mkay?
			if (strlen(buf)>4) {
				get_user_input(term_buf,&term_buflen,buf,len);
				done=1;
				break;
			}
			switch(buf[2]) {
				case '1': // HOME KEY
				term_curpos=-term_buflen;
				move_for_pow(term_curpos);
				done=1;
				break;
				case '2': // INSERT KEY
					if (insert_mode) {
						insert_mode=0;
					} else {
						insert_mode=1;
					}
				done=1;
				break;
				
				case '3': // DELETE KEY
					insch(term_curpos); 

					if (term_curpos<0) {
						termbuf_delete_at(term_curpos);
						term_curpos++;
						
					}
				done=1;
					break;
 
				case '4': // END KEY
				move_for_pow(abs(term_curpos));
				term_curpos=0;
				done=1;
				break;
				case '5': // PG UP
				case '6': // PGDN
				case 'A': // UP
				case 'B': // DOWN
				get_user_input(term_buf,&term_buflen,buf,len);
				done=1;
				break;
				case 'C': // RIGHT
						if (term_curpos<0) {
						term_curpos+=1;
						move_for_pow(1);
						}
					done=1;
				break;

				case 'D': // LEFT
					if (term_buflen>=abs(term_curpos-1)) {
						term_curpos-=1;
						move_for_pow(-1);
					}
					done=1;
				break;
		
				default:
				sprintf(errtext,"\r\nis: %s",&buf[2]);
				from_backend(0,errtext,strlen(errtext));
				break;

			}
		break;

		case 'O':
			// let powwow deal with keypad bindings

			get_user_input(term_buf,&term_buflen,buf,len);
			done=1;
			break;


			

		}

		if (done) return;
	}


	
	
    while (len--) {

	char c;
        c = *buf++;

	switch (term_quotenext ? ' ' : c) {
	    /*
	     * ^h/^?: delete one char and output one BSB
	     * ^w: delete, and output BSBs, to return to last space/nonspace
	     * boundary
	     * ^u: delete, and output BSBs, to return to BOL
	     * ^c: Do a ^u then send a telnet IP
	     * ^z: Do a ^u then send a telnet SUSP
	     * ^\: Do a ^u then send a telnet ABORT
	     * ^r: echo "^R\n" and redraw line
	     * ^v: quote next char
	     * ^d: if at BOL, end of file and close connection, else send line
	     * and reset to BOL
	     * ^m: send line-plus-\r\n and reset to BOL
	     */

	case CTRL('K'):
		insch(term_curpos);

		term_buflen+=term_curpos;
		term_curpos=0;
		break;
		
	case CTRL('A'):
			move_for_pow(abs(term_curpos));
			// move_for_pow(-(term_buflen+term_curpos));
			term_curpos=-term_buflen;
			move_for_pow(term_curpos);
			

		break;

			
	case CTRL('E'):
			move_for_pow(abs(term_curpos));
			term_curpos=0;
			

		break;

	   case CTRL('?'): /* backspace! */ 
		   if (term_buflen) {
			   if (term_curpos<0) {
					if (abs(term_curpos)< term_buflen) {
					insch(term_curpos-1);
					termbuf_delete_at(term_curpos-1);
					move_for_pow(-1);
					}
			   } else if (term_curpos==0) {
				   	termbuf_delete_at(term_curpos);
					bsb(1);
					move_for_pow(-1);
			   }
		   }

		 break;
	   case CTRL('H'): /* shift-backspace */
		   	insch(term_curpos); 

			if (term_curpos<0) {
				termbuf_delete_at(term_curpos);
				term_curpos++;
						
			}
			break;

	  case CTRL('W'):		       /* delete word */
	    while (term_buflen > 0) {
		bsb(plen(term_buf[term_buflen-1]));
		term_buflen--;
		if (term_buflen > 0 &&
		    isspace(term_buf[term_buflen-1]) &&
		    !isspace(term_buf[term_buflen]))
		    break;
	    }
	    break;
	  case CTRL('U'):	       /* delete line */
		  	   while (term_buflen > 0) {
				bsb(plen(term_buf[term_buflen-1]));
				term_buflen--;
				}
		break;
	  case CTRL('I'): // tab key!
		get_user_input(term_buf,&term_buflen,"\t",1);
		break;
	  case CTRL('V'):
			term_mouse (MB_PASTE, MA_CLICK, 0, 0);
            term_mouse (MB_PASTE, MA_RELEASE, 0, 0);
		  break;
	  case CTRL('C'):	       
		  // do nothing.. let them think that they are using this for copy
		  break;
	  case CTRL('Z'):	       /* Suspend */
	    while (term_buflen > 0) {
		bsb(plen(term_buf[term_buflen-1]));
		term_buflen--;
	    }
	    back->special (TS_EL);
	    if( c == CTRL('C') )  back->special (TS_IP);
	    if( c == CTRL('Z') )  back->special (TS_SUSP);
	    if( c == CTRL('\\') ) back->special (TS_ABORT);
            break;
	  case CTRL('R'):	       /* redraw line */
	    c_write("^R\r\n", 4);
	    {
		int i;
		for (i = 0; i < term_buflen; i++)
		    pwrite(term_buf[i]);
	    }
	    break;
	  case CTRL('D'):	       /* logout or send */
	    if (term_buflen == 0) {
		back->special (TS_EOF);
	    } else {
		back->send(term_buf, term_buflen);
		term_buflen = 0;
	    }
	    break;
	  case CTRL('M'):	       /* send with newline */
		    last_edit_cmd = (function_str)0;

		  if (term_buflen>0) {
			  int i;
			term_buf[term_buflen]='\0';
		
			for (i=1;i<term_buflen;i++) {
				if ((term_buf[0] !='#') && (term_buf[i]=='\\')) { // if we hit an escape, check next char
					if (i+1<term_buflen) {
						if (term_buf[i+1] !=124) { // then escape anything that is not ;
							int j;
							term_buflen++;
							for (j=term_buflen;j>i;j--) {
									term_buf[j]=term_buf[j-1];
								}
							// term_buf[i]='\\';
							i+=1;
							
							}
					} else {
						term_buf[term_buflen++]='\\';
						term_buf[term_buflen]='\0';
						i++;
					}
				}
			}

			prompt_status=1;
			c_write("\r\n", 2);

			parse_user_input (term_buf,1);
			to_history(term_buf);

			draw_prompt();

		  } else {
			c_write("\r\n", 2);
			back->send("\n",1);
		  }
		term_curpos=0;

		term_buflen=0;	
		break;
	  default:                     /* get to this label from ^V handler */

	    if (term_buflen >= term_bufsiz) {
		term_bufsiz = term_buflen + 256;
		term_buf = saferealloc(term_buf, term_bufsiz);
	    }

		if (c !=' ' && c !='~') {
			get_user_input(term_buf,&term_buflen,&c,len);	
		}

		// if we're at the end of the normal input line, just add it and print it
		if (term_curpos==0) {
			term_buf[term_buflen++] = c;
		
			pwrite(c);
		} else {
			if (insert_mode) {
				int i;

				// make room in the buffer for an insert
				// potential bug: if you break 1024!
				term_buflen++;

			
				// shift everything forward by one char
				// and output as we go

				for (i=term_buflen;i>=term_buflen+term_curpos;i--) {
					term_buf[i]=term_buf[i-1];
				}

				term_buf[term_buflen+term_curpos-1]=c;

				for (i=term_buflen+term_curpos-1;i<term_buflen;i++) {
					pwrite(term_buf[i]);

				}
				

			} else {

				// overtype! and actually do it

				term_buf[term_buflen+term_curpos]=c;
				pwrite(c);
				term_out(0);
				// term_out triggers a move back, so we have to move forward again
				// for some reason, calling term_out(1) messes it all up

				move_for_pow(-term_curpos); 
				

			}

		}
		
            term_quotenext = FALSE;

	    break;

	}
    }

}

static void simple_send(char *buf, int len) {
    if( term_buflen != 0 )
    {
	back->send(term_buf, term_buflen);
	while (term_buflen > 0) {
	    bsb(plen(term_buf[term_buflen-1]));
	    term_buflen--;
	}
    }
    if (len > 0)
        back->send(buf, len);
}

void clear_input_line(int junk) {

	if (term_curpos<0) {
		move_for_pow(-term_curpos);
	}

	if (term_buflen && !has_cleared) {
	bsb(term_buflen);
 // this big if is to fix a bug where powtty puts an extra line randomly!

	if ((last_printed !='\n') &&
		(last_printed !='\r') &&
		(last_printed !='\t') && // this one added as a hack/check for "/account"
		(last_printed != 0)) {
			from_backend(0,"\r\n",2);
		}
	
	has_cleared=1;
	}


}
void redraw_input_line() {
	if (term_buflen >0)
	{
		from_backend(0,term_buf,term_buflen);
		has_cleared=0;
	}


}

void update_term_buf(char *buf,int len) {
// for some reason insch fails on this routine
// so I used bsb :(

	int i;

	if (term_curpos<0) {
		move_for_pow(-term_curpos);
		term_curpos=0;
	}

	if (len>term_buflen) {
		insch((len-term_buflen));
	}
	if (term_buflen>0) {
		bsb(term_buflen);
		term_buflen=0;
	}

	term_buflen=len;
	strcpy(term_buf,buf);

	for (i=0;i<term_buflen;i++) {
		pwrite(term_buf[i]);
	}
	
}

void add_term_buf (char *buf,int len) {
	int i=0;
	if (term_curpos==0) {
		for (i=0;i<len;i++) {
			if (buf[i] != '\0') {
				term_buf[term_buflen++]=buf[i];
				pwrite(buf[i]);
			}

		}
	} else {

		for (i=0;i<len;i++) {
			if (buf[i] !='\0') {
				term_buf[term_buflen+term_curpos+i]=buf[i];
				
				if (term_buflen+term_curpos+i > term_buflen) {
					term_buflen++;
				}
				pwrite(buf[i]);
				move_for_pow(-term_curpos); 
			}


			}
	}
	term_buf[term_buflen]='\0';

}


void input_delete_nofollow_chars(int n) {
	int i;

	for (i=0;i<n;i++) {
		if (term_curpos<0) {
			
					if (abs(term_curpos)< term_buflen) {
					insch(term_curpos-1);
					termbuf_delete_at(term_curpos-1);
					move_for_pow(-1);
					}
					term_curpos++;

			   } else if (term_curpos==0) {
					insch(term_curpos-1);
					termbuf_delete_at(term_curpos);
					bsb(1);
					move_for_pow(-1);
					term_buf[term_buflen]='\0';
			   }
	}
}



Ldisc ldisc_term = { term_send };
Ldisc ldisc_simple = { simple_send };


