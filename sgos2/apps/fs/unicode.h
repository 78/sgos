#ifndef _UNICODE_H_
#define _UNICODE_H_

#include <sgos.h>

typedef ushort wchar_t;

int unicode_init();
int unicode_decode( wchar_t* input, int inputLen, char* output, int outputLen );
int unicode_encode( unsigned char* input, int inputLen, wchar_t* output, int outputLen );

#endif
