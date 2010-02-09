#include <sgos.h>
#include <arch.h>

t_16 ArInWord( t_16 port )
{
	register t_16 ret;
	__asm__ __volatile__ ( "in %%dx , %%ax" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}

t_32 ArInWord32( t_16 port )
{
	register t_16 ret;
	__asm__ __volatile__ ( "in %%dx , %%eax" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}

t_8 ArInByte( t_16 port )
{
	register t_8 ret;
	__asm__ __volatile__ ( "in %%dx , %%al" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}
