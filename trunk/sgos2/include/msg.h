#ifndef _MSG_H_
#define _MSG_H_

#include <sgos.h>
#include <bxml.h>

#define MIN_RECV_BUFFER_SIZE	1024

typedef struct MESSENGER{
	bxml_t*	bxml;
	char	dest_name[NAME_LEN];
	uint	dest_thread;
	session_t	session;
	char*	buffer;
	size_t	buffer_size;
}messenger_t;

#ifdef __cplusplus 
extern "C" {
#endif

//Namespace
int create_name( uint thread, const char* name );
int delete_name( uint thread, const char* name );
uint match_name( const char* name );

// 信使初始化
void	msg_init( messenger_t * msger );
// 解析一个消息文本
int	msg_parse( messenger_t * msger, const char* str );
// 创建一个路径，并且转到创建的路径
int	msg_mkdir( messenger_t * msger, const char* path );
// 追加一个节点
int	msg_append( messenger_t * msger, const char* name );
// 重定位当前路径
int	msg_redir( messenger_t * msger, const char* path );
// 移动到下一项
int	msg_movenext( messenger_t * msger );
// 提交一个同步请求
int	msg_request( messenger_t * msger );
// 发送消息，异步
int	msg_send( messenger_t* msger, uint flag );
// 接收消息
int	msg_recv( messenger_t* msger, uint flag );
// 接收消息，指定线程
int	msg_recv_ex( messenger_t* msger, uint thread, uint flag );
// 创建下一项
// not implemented
void	msg_cleanup( messenger_t * msger );
// 读操作
int 	msg_read( messenger_t * msger, const char* path, void* buf, uint buf_size );	//read 
// 读节点名
char* 	msg_readname( messenger_t * msger, const char* path );
// 写操作
int 	msg_write( messenger_t * msger, const char* path, const void* buf, uint buf_size );	//write 
// 读取一个字符串
char*	msg_get_str( messenger_t * msger, const char* path );
int	msg_get_int( messenger_t * msger, const char* path );
uint	msg_get_uint( messenger_t * msger, const char* path );
short	msg_get_short( messenger_t * msger, const char* path );
ushort	msg_get_ushort( messenger_t * msger, const char* path );
char	msg_get_char( messenger_t * msger, const char* path );
uchar	msg_get_uchar( messenger_t * msger, const char* path );
// 写入一个字符串
int	msg_put_str( messenger_t* msger, const char* path, const char* str );
int	msg_put_int( messenger_t* msger, const char* path, int v );
int	msg_put_uint( messenger_t* msger, const char* path, uint v );
int	msg_put_short( messenger_t* msger, const char* path, short v );
int	msg_put_ushort( messenger_t* msger, const char* path, ushort v );
int	msg_put_char( messenger_t* msger, const char* path, char v );
int	msg_put_uchar( messenger_t* msger, const char* path, uchar v );

#ifdef __cplusplus 
}
#endif


#endif
