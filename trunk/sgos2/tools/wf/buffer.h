#ifndef _BUFFER_H_
#define _BUFFER_H_

#define BLOCK_SIZE 1024
typedef unsigned int t_32;
typedef unsigned short t_16;
typedef unsigned char t_8;

int buffer_init( const char* fname );
void* buffer_read( unsigned short dev, unsigned int block );
int buffer_write( unsigned short dev, unsigned int block, void* datas );
void buffer_release( void* );
void buffer_cleanup();

#endif
