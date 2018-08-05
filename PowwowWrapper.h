// added by billj - bridges

#ifndef POWWOW_WRAPPER
#define POWWOW_WRAPPER

char *powgetenv(LPTSTR);
int isConnectedtoServer();
int tcp_raw_write (); // this is for the id thing to happen
void tcp_write (); 
void tty_write(int start,char *arg,int len); 
void tty_raw_write(char *arg,int len); 
int tty_read (char *arg,int chunk);
void powexit(int code);
int tcp_read(char *arg,int len); // used by beam.c
void tty_flush();
#endif

