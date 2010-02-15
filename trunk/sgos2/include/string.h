#ifndef _STRING_H_
#define _STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif




#ifdef __cplusplus 
extern "C" char * strerror(int errno);
extern "C" char * ___strtok;
extern "C" {
#else
extern char * strerror(int errno);
extern char * ___strtok;
#endif

char * strcpy(char * dest,const char *src);
char * strncpy(char * dest,const char *src,int count);
char * strcat(char * dest,const char * src);
char * strncat(char * dest,const char * src,int count);
int strcmp(const char * cs,const char * ct);
int strncmp(const char * cs,const char * ct,int count);
char * strchr(const char * s,int c);
char * strrchr(const char * s,int c);
int strspn(const char * cs, const char * ct);
int strcspn(const char * cs, const char * ct);
char * strpbrk(const char * cs,const char * ct);
char * strstr(const char * cs,const char * ct);
char * strlwr(char * str);
int strlen(const char * s);

char * strtok(char * s,const char * ct);
void * memcpy(void * dest,const void * src, int n);
void * memcpyw(void * dest,const void * src, int n);
void * memcpyd(void * dest,const void * src, int n);
char * memmove(char * dest,const char * src, int n);
int memcmp(const void * cs,const void * ct,int count);
void * memchr(const void * cs,char c,int count);
void * memset(void * s,int c,int count);
void * memsetw(void * s,short n,int count);
void * memsetd(void * s,int n,int count);

int strnicmp(char * s1, char * s2, int len);
int stricmp(char * s1, char * s2);

#ifdef __cplusplus 
}
#endif

#endif
