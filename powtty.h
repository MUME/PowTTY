/* public definitions from tty.c */

#ifndef _TTY_H_
#define _TTY_H_

extern int tty_read_fd;

void tty_bootstrap	(void);
void tty_start		(void);
void tty_quit		(void);
void tty_special_keys	(void);
void tty_sig_winch_bottomhalf	(void);
void tty_add_walk_binds		(void);
void tty_add_initial_binds	(void);

void tty_gets		(char *s, int size);
void tty_raw_write	(char *data, int len);
#endif /* _TTY_H_ */
