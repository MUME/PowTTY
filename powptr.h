/*
 *  ptr.h -- type definitions for ptr (aka `pointer'), a char* replacement
 *           which allows for '\0' inside a string.
 */

#ifndef _PTR_H_
#define _PTR_H_

typedef struct s_ptr {
    int len;
    int max;
} _ptr;

typedef _ptr * ptr;

#define sizeofptr ((int)(1 + sizeof(_ptr)))

/* the + 0 below is to prohibit using the macros for altering the ptr */
#define ptrlen(p) ((p)->len + 0)
#define ptrmax(p) ((p)->max + 0)
#define ptrdata(p) ((char *)((ptr)(p) + 1))
/* if p is a valid (ptr), ptrdata(p) is guaranteed to be a valid (char *) */

ptr   ptrnew    (int max);
ptr   ptrdup2   (ptr src, int newmax);
ptr   ptrdup    (ptr src);
void  ptrdel    (ptr p);

void  ptrzero   (ptr p);
void  ptrshrink (ptr p, int len);
void  ptrtrunc  (ptr p, int len);
ptr   ptrpad    (ptr p, int len);
ptr   ptrsetlen (ptr p, int len);

ptr   ptrcpy    (ptr dst, ptr src);
ptr   ptrmcpy   (ptr dst, char *src, int len);

ptr   ptrcat    (ptr dst, ptr src);
ptr   ptrmcat   (ptr dst, char *src, int len);


ptr __ptrcat    (ptr dst, char *src, int len, int shrink);
ptr __ptrmcpy   (ptr dst, char *src, int len, int shrink);

int   ptrcmp    (ptr p, ptr q);
int   ptrmcmp   (ptr p, char *q, int lenq);

char *ptrchr    (ptr p, char c);
char *ptrrchr   (ptr p, char c);

char *ptrfind    (ptr p, ptr q);
char *ptrmfind   (ptr p, char *q, int lenq);

char *ptrchrs   (ptr p, ptr q);
char *ptrmchrs  (ptr p, char *q, int lenq);
char *ptrrchrs  (ptr p, ptr q);
char *ptrmrchrs (ptr p, char *q, int lenq);

char *memrchr   (char *p, int lenp, char c);
char *memchrs   (char *p, int lenp, char *q, int lenq);
char *memrchrs  (char *p, int lenp, char *q, int lenq);
#ifdef _GNU_SOURCE
# define memfind memmem
#else
char *memfind   (char *hay, int haylen, char *needle, int needlelen);
#endif

#endif /* _PTR_H_ */
