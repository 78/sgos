#ifndef _UNICODE_H_
#define _UNICODE_H_

#ifdef __WIN32__
typedef unsigned short wchar_t;
#endif

int unicode_init();
int unicode_decode( wchar_t* input, int inputLen, char* output, int outputLen );
int unicode_encode( unsigned char* input, int inputLen, wchar_t* output, int outputLen );

#endif
