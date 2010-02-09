// bits
#ifndef _MMOP_H_
#define _MMOP_H_

void * memcpy8(void * dest, const void * src, int n);
void * memcpy16(void * dest, const void * src, int n);
void * memcpy32(void * dest, const void * src, int n);
void * memset8(void * s, unsigned char c, int count);
void * memset16(void * s, unsigned short n, int count);
void * memset32(void * s, unsigned int n, int count);

#endif
