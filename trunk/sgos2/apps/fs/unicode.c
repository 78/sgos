//Huang Guan
//Unicode
//2007-8-16 22:44 Created
//gdxxhg@gmail.com

#include <stdio.h>
#include <stdlib.h>
#include "unicode.h"

#define UNICODE_DATABASE "unicode.db"
static char* unicodeDB;

static int LoadUnicodeDatabase(const char* fileName)
{
	int len;
	unicodeDB = (char*)0;
	return 0;
}

int unicode_init()
{
	if( !unicodeDB )
		LoadUnicodeDatabase( UNICODE_DATABASE );
	return 0;
}

int unicode_decode( wchar_t* input, int inputLen, char* output, int outputLen )
{
	int i, j;
	for( i=0, j=0; i+1<inputLen && input[i] && j<outputLen; i++ )
	{
		if( input[i]<0x80 )
		{
			output[j++] = (char)input[i];
		}else{
			if( !unicodeDB )
			{
				//PERROR("ERROR Loading Unicode db.");
				continue;
			}
			char* ch = unicodeDB + ((input[i]-0x4E00)<<1);
			output[j++] = *ch++;
			output[j++] = *ch;
			unsigned char a = (*(ch-1))&0xff;
			unsigned char b = (*ch)&0xff;
		}
	}
	output[j] = '\0';
	return j;
}

int unicode_encode( unsigned char* input, int inputLen, wchar_t* output, int outputLen )
{
	int i, j;
	for( i=0, j=0; i<inputLen && input[i] && j+1<outputLen; i++ )
	{
		if( input[i]<0x80 )
		{
			wchar_t w = (wchar_t)input[i];
			output[j++] = w&0x7F;
		}else{
			if( !unicodeDB )
			{
				//PERROR("ERROR Loading Unicode db.");
				continue;
			}
			output[j++] = (wchar_t)'_';
		}
	}
	output[j] = '\0';
	return j;
}
