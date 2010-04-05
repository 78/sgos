#ifndef _IO_H
#define _IO_H

//IO操作
uchar inbyte( ushort port );
ushort inword( ushort port );
uint indword( uint port );
void outbyte( ushort port, uchar v );
void outword( ushort port, ushort v );
void outdword( ushort port, uint v );
#endif
