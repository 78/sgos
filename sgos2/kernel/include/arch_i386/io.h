#ifndef _IO_H_
#define _IO_H_

// (端口, 数据)
#define out_byte_wait(port,data) \
__asm__ __volatile__ ( "out %%al , %%dx\n\tjmp 1f\n1:\tjmp 1f\n1:" : : "a"( data ) , "d"( port ) );
#define out_word_wait(port,data) \
__asm__ __volatile__ ( "out %%ax , %%dx\n\tjmp 1f\n1:\tjmp 1f\n1:" : : "a"( data ) , "d"( port ) );
#define out_byte(port,data) \
__asm__ __volatile__ ( "out %%al , %%dx" : : "a"( data ) , "d"( port ) );
#define out_word(port,data) \
__asm__ __volatile__ ( "out %%ax , %%dx" : : "a"( data ) , "d"( port ) );
#define out_dword(port,data) \
__asm__ __volatile__ ( "out %%eax , %%dx" : : "a"( data ) , "d"( port ) );



// port.c
t_16 in_word( t_16 port );
t_32 in_dword( t_16 port );
t_8 in_byte( t_16 port );

#endif
