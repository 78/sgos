//Huang Guan
//090803

#include <sgos.h>
#include <multiboot.h>

char ss[3453];

void multiboot_init( uint magic, uint addr )
{
	ushort* t = (ushort*)0xC00B8000;
	*t++ = 0xAAAA;
	*t++ = 0x1111;
	*t++ = 0x2222;
}

