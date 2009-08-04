#include <sgos.h>
#include <arch.h>

t_16 in_word( t_16 port )
{
	register t_16 ret;
	__asm__ __volatile__ ( "in %%dx , %%ax" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}

t_32 in_dword( t_16 port )
{
	register t_16 ret;
	__asm__ __volatile__ ( "in %%dx , %%eax" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}

t_8 in_byte( t_16 port )
{
	register t_8 ret;
	__asm__ __volatile__ ( "in %%dx , %%al" : "=a"( ret ) : "d"( port ) ) ;
	return ret;
}
