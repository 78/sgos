
#include <sgos.h>
#include <string.h>
#include <msg.h>
#include <bxml.h>

// 读取一个字符串
char*	msg_get_str( messenger_t * msger, const char* path )
{
	if( !msger->bxml )
		return NULL;
	return bxml_readstr( msger->bxml, path );
}

int	msg_get_int( messenger_t * msger, const char* path )
{
	int ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

uint	msg_get_uint( messenger_t * msger, const char* path )
{
	uint ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

short	msg_get_short( messenger_t * msger, const char* path )
{
	short ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

ushort	msg_get_ushort( messenger_t * msger, const char* path )
{
	ushort ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

char	msg_get_char( messenger_t * msger, const char* path )
{
	char ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

uchar	msg_get_uchar( messenger_t * msger, const char* path )
{
	uchar ret=0;
	msg_read( msger, path, &ret, sizeof(ret) );
	return ret;
}

// 写入一个字符串
int	msg_put_str( messenger_t* msger, const char* path, const char* str )
{
	if( !msger->bxml )
		return -ERR_NOINIT;
	bxml_writestr( msger->bxml, path, str );
	return 0;
}

int	msg_put_int( messenger_t* msger, const char* path, int v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}

int	msg_put_uint( messenger_t* msger, const char* path, uint v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}

int	msg_put_short( messenger_t* msger, const char* path, short v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}

int	msg_put_ushort( messenger_t* msger, const char* path, ushort v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}

int	msg_put_char( messenger_t* msger, const char* path, char v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}

int	msg_put_uchar( messenger_t* msger, const char* path, uchar v )
{
	return msg_write( msger, path, &v, sizeof(v) );
}
