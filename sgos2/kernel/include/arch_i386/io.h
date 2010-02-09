#ifndef _IO_H_
#define _IO_H_

// (端口, 数据)
#define ArOutByteWait(port,data) \
__asm__ __volatile__ ( "out %%al , %%dx\n\tjmp 1f\n1:\tjmp 1f\n1:" : : "a"( data ) , "d"( port ) );
#define ArOutWordWait(port,data) \
__asm__ __volatile__ ( "out %%ax , %%dx\n\tjmp 1f\n1:\tjmp 1f\n1:" : : "a"( data ) , "d"( port ) );
#define ArOutByte(port,data) \
__asm__ __volatile__ ( "out %%al , %%dx" : : "a"( data ) , "d"( port ) );
#define ArOutWord(port,data) \
__asm__ __volatile__ ( "out %%ax , %%dx" : : "a"( data ) , "d"( port ) );
#define ArOutWord32(port,data) \
__asm__ __volatile__ ( "out %%eax , %%dx" : : "a"( data ) , "d"( port ) );



// port.c
t_16 ArInWord( t_16 port );
t_32 ArInWord32( t_16 port );
t_8 ArInByte( t_16 port );

#endif
