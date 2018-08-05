/* public things from edit.c */

#ifndef _EDIT_H_
#define _EDIT_H_

typedef struct {
  char *name;
  function_str funct;
} edit_function;

extern edit_function internal_functions[];

/* 
 * GH: completion list, stolen from cancan 2.6.3a
 *	
 *     words[wordindex] is where the next word will go (always empty)
 *     words[wordindex].prev is last (least interesting)
 *     words[wordindex].next is 2nd (first to search for completion)
 */
#define WORD_UNIQUE	1		/* word is unique in list */
#define	WORD_RETAIN	2		/* permanent (#command) */
typedef struct {
    char *word;
    int  next, prev;
    char flags;
} wordnode;

extern char *hist[MAX_HIST];
extern int curline;
extern int pickline;

extern wordnode words[MAX_WORDS];
extern int wordindex;

/*         public function declarations         */
void edit_bootstrap	   (void);

int  lookup_edit_name	   (char *name, char **arg);
int  lookup_edit_function  (function_str funct);
void to_history		 (char *dummy);
void put_history	 (char *str);
void complete_word	 (char *dummy);
void complete_line	 (char *dummy);
void put_word		 (char *s);
void set_custom_delimeters  (char *s);
void to_input_line	 (char *str);
void clear_line		 (char *dummy);
void enter_line		 (char *dummy);
void prev_line		 (char *dummy);
void next_line		 (char *dummy);
void key_run_command	 (char *cmd);
void draw_prompt (void);

#endif /* _EDIT_H_ */
