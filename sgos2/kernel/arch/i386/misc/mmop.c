// mmop.c 
// bits

#include <mmop.h>

void * memcpy8(void * dest,const void * src, int n)
{
__asm__("cld\n\t"
	"rep\n\t"
	"movsb"
	::"c" (n),"S" (src),"D" (dest));
return dest;
}

void * memcpy16(void * dest,const void * src, int n)
{
__asm__("cld\n\t"
	"rep\n\t"
	"movsw"
	::"c" (n),"S" (src),"D" (dest));
return dest;
}

void * memcpy32(void * dest,const void * src, int n)
{
__asm__("cld\n\t"
	"rep\n\t"
	"movsl"
	::"c" (n),"S" (src),"D" (dest));
return dest;
}


void * memset8(void * s,unsigned char c,int count)
{
__asm__("cld\n\t"
	"rep\n\t"
	"stosb"
	::"a" (c),"D" (s),"c" (count));
return s;
}

void * memset16(void * s,unsigned short n,int count)
{
__asm__("cld\n\t"
	"rep\n\t"
	"stosw"
	::"a" (n),"D" (s),"c" (count));
return s;
}

void * memset32(void * s,unsigned int n,int count)
{
__asm__("cld\n\t"
	"rep\n\t"
	"stosl"
	::"a" (n),"D" (s),"c" (count));
return s;
}
