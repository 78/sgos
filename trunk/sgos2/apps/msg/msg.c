// Messenger work

#include <sgos.h>
#include <stdlib.h>
#include <string.h>
#include <msg.h>
#include <api.h>
#include <bxml.h>


// Get the messenger from thread information
static messenger_t* get_messenger()
{
	messenger_t* msger;
	THREAD_INFO* ti = thread_info();
	msger = ti->messenger;
	if( !msger )
		ti->messenger = msger = (messenger_t*)
			malloc(sizeof(messenger_t));
	return msger;
}

// 信使初始化
void	msg_init( messenger_t * msger )
{
	memset( msger, 0, sizeof(messenger_t));
}

// 解析一个消息文本
int	msg_parse( messenger_t * msger, const char* str )
{
	if( msger->bxml )
		bxml_free( msger->bxml );
	msger->bxml = bxml_parse( str );
	if( !msger->bxml )
		return -ERR_NOMEM;
	return 0;
}

// 创建一个路径，并且转到创建的路径
int	msg_mkdir( messenger_t * msger, const char* path )
{
	if(!msger->bxml)
		return -ERR_NOINIT;
	return bxml_redirect( msger->bxml, path, 1 );
}

// 重定位当前路径
int	msg_redir( messenger_t * msger, const char* path )
{
	if(!msger->bxml)
		return -ERR_NOINIT;
	return bxml_redirect( msger->bxml, path, 0 );
}

// 移动到下一项
int	msg_movenext( messenger_t * msger )
{
	if(!msger->bxml)
		return 0; //也是没有下一项
	return bxml_movenext( msger->bxml );
}

// 追加一个节点
int	msg_append( messenger_t * msger, const char* name )
{
	if(!msger->bxml)
		return 0; //也是没有下一项
	return bxml_mknode( msger->bxml, name );
}

// 提交一个同步请求
int	msg_request( messenger_t * msger )
{ 
	int ret;
	ret = msg_send( msger, 0 );
	if( ret<0 )
		return ret;
	ret = msg_recv_ex( msger, msger->dest_thread, MSG_PENDING );
	if( ret<0 )
		return ret;
	return 0;
}

// 发送消息，异步
int	msg_send( messenger_t* msger, uint flag )
{
	const char * dest_name;
	size_t len;
	if(!msger->bxml)
		return -ERR_NOINIT;
	dest_name = bxml_readstr( msger->bxml, "/:to" );
	if( dest_name ){
		if( !(*msger->dest_name) || strcmp(msger->dest_name, dest_name )!=0 ){
			//get the thread
			msger->dest_thread = sys_namespace_match( (char*)dest_name );
			if( !msger->dest_thread ){
				return -ERR_NODEST;
			}
			strcpy( msger->dest_name, dest_name );
		}
	}else{
		//reply??
		msger->dest_thread = msger->session.thread;
	}
	msger->session.thread = msger->dest_thread;
	len = bxml_buffer_size( msger->bxml );
	if( !msger->buffer || msger->buffer_size < len ){
		if( msger->buffer )
			free( msger->buffer );
		msger->buffer_size = len;
		msger->buffer = malloc( msger->buffer_size );
	}
	bxml_build( msger->bxml, msger->buffer, msger->buffer_size );
	return sys_send( &msger->session, msger->buffer, len, flag );
}

// 接收消息
int	msg_recv( messenger_t* msger, uint flag )
{
	return msg_recv_ex( msger, 0, flag );
}

// 接收消息，指定线程
int	msg_recv_ex( messenger_t* msger, uint thread, uint flag )
{
	const char * dest_name;
	size_t len;
	int ret;
	if( msger->buffer_size < MIN_RECV_BUFFER_SIZE ){
		free( msger->buffer );
		msger->buffer_size = MIN_RECV_BUFFER_SIZE;
		msger->buffer = malloc( msger->buffer_size );
	}
	len = msger->buffer_size;
	if( thread )
		msger->session.thread = thread;
	else
		msger->session.thread = 0;
	ret = sys_recv( &msger->session, msger->buffer, &len, flag );
	if( ret < 0 ){
		if( -ret == ERR_NOBUF ){
			free( msger->buffer );
			msger->buffer_size = len;
			msger->buffer = malloc( msger->buffer_size );
			ret = sys_recv( &msger->session, msger->buffer, &len, flag );
		}else{
			return ret;
		}
	}
	if( msger->bxml )
		bxml_free(msger->bxml);
	msger->bxml = bxml_parse( msger->buffer );
	if( msger->bxml == NULL )
		return -ERR_UNKNOWN;
	return ret;
}

// 创建下一项
// not implemented
void	msg_cleanup( messenger_t * msger )
{
	if( msger->bxml )
		bxml_free( msger->bxml );
	if( msger->buffer )
		free( msger->buffer );
	msger->bxml = NULL;
	msger->buffer = NULL;
}

// 读操作
int 	msg_read( messenger_t * msger, const char* path, void* buf, uint buf_size )
{
	if(!msger->bxml)
		return -ERR_NOINIT;
	return bxml_read( msger->bxml, path, buf, buf_size );
}

// 读节点名操作
char* 	msg_readname( messenger_t * msger, const char* path )
{
	if(!msger->bxml)
		return NULL;
	return bxml_readname( msger->bxml, path );
}


// 写操作
int 	msg_write( messenger_t * msger, const char* path, const void* buf, uint buf_size )
{
	if(!msger->bxml)
		return -ERR_NOINIT;
	bxml_write( msger->bxml, path, buf, buf_size );
	return 0;
}
