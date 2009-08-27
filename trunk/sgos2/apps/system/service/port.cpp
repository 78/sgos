// X86 IO Operations
#include <system.h>

namespace System{
	namespace Service{

		//IO操作
		uchar inbyte( ushort port )
		{
			register t_8 ret;
			__asm__ __volatile__ ( "in %%dx , %%al" : "=a"( ret ) : "d"( port ) ) ;
			return ret;
		}
		
		ushort inword( ushort port )
		{
			register t_16 ret;
			__asm__ __volatile__ ( "in %%dx , %%ax" : "=a"( ret ) : "d"( port ) ) ;
			return ret;
		}
		
		uint indword( uint port )
		{
			register t_16 ret;
			__asm__ __volatile__ ( "in %%dx , %%eax" : "=a"( ret ) : "d"( port ) ) ;
			return ret;
		}
		
		void outbyte( ushort port, uchar v )
		{
			__asm__ __volatile__ ( "out %%al , %%dx" : : "a"( v ) , "d"( port ) );
		}
		
		void outword( ushort port, ushort v )
		{
			__asm__ __volatile__ ( "out %%ax , %%dx" : : "a"( v ) , "d"( port ) );
		}
		
		void outdword( ushort port, uint v )
		{
			__asm__ __volatile__ ( "out %%eax , %%dx" : : "a"( v ) , "d"( port ) );
		}
	}
}
