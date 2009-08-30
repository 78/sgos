#include <sgos.h>
#include <string.h>
#include <stdio.h>
#include <system.h>
#include <msg.h>

namespace System{
	Messenger::Messenger()
	{
		msg_init( &msg );
		flag = 0;
	}

	Messenger::Messenger( const char* str )
	{
		msg_init( &msg );
		flag = 0;
		parse( str );
	}

	Messenger::~Messenger()
	{
		msg_cleanup( &msg );
	}
	
	// 回复
	Messenger Messenger::reply()
	{
		Messenger m;
		m.msg.recv_thread = this->msg.recv_thread;
		return m;
	}
	
	//
	Messenger& Messenger::append( const char* name )
	{
		msg_append( &msg, name );
		return *this;
	}

	// 解析一个消息文本
	int Messenger::parse( const char* str )
	{
		return msg_parse( &msg, str );
	}

	// 创建一个路径，并且转到创建的路径
	int Messenger::mkdir( const char* path )
	{
		return msg_mkdir( &msg, path );
	}

	// 重定位当前路径
	int Messenger::redir( const char* path )
	{
		return msg_redir( &msg, path );
	}

	// 移动到下一项
	int Messenger::moveNext()
	{
		return msg_movenext( &msg );
	}

	// 提交一个同步请求
	int Messenger::request()
	{
		return msg_request( &msg );
	}
		
	// 发送消息，异步
	int Messenger::send()
	{
		return msg_send( &msg, flag );
	}

	// 接收消息
	int Messenger::receive(bool pending)
	{
		if( pending ){
			return msg_recv( &msg, flag|MSG_PENDING );
		}else{
			return msg_recv( &msg, flag );
		}
	}

	// 读操作
	int Messenger::read( const char* path, void* buf, uint buf_size )
	{
		return msg_read( &msg, path, buf, buf_size );
	}

	// 读节点名操作
	const char* Messenger::readName( const char* path )
	{
		const char* ret = msg_readname( &msg, path );
		return ret;
	}

	// 写操作
	int Messenger::write( const char* path, const void* buf, uint buf_size )
	{
		return msg_write( &msg, path, buf, buf_size );
	}

	// 读取一个字符串
	char* Messenger::getString( const char* path )
	{
		return msg_get_str( &msg, path );
	}

	int Messenger::getInt( const char* path )
	{
		return msg_get_int( &msg, path );
	}

	uint Messenger::getUInt( const char* path )
	{
		return msg_get_uint( &msg, path );
	}

	short Messenger::getShort( const char* path )
	{
		return msg_get_short( &msg, path );
	}

	ushort Messenger::getUShort( const char* path )
	{
		return msg_get_ushort( &msg, path );
	}

	char Messenger::getChar( const char* path )
	{
		return msg_get_char( &msg, path );
	}

	uchar Messenger::getUChar( const char* path )
	{
		return msg_get_uchar( &msg, path );
	}

	uchar Messenger::getByte( const char* path )
	{
		return msg_get_uchar( &msg, path );
	}

	// 写入一个字符串
	int Messenger::putString( const char* path, const char* str )
	{
		return msg_put_str( &msg, path, str );
	}

	int Messenger::putInt( const char* path, int v )
	{
		return msg_put_int( &msg, path, v );
	}

	int Messenger::putUInt( const char* path, uint v )
	{
		return msg_put_uint( &msg, path, v );
	}

	int Messenger::putShort( const char* path, short v )
	{
		return msg_put_short( &msg, path, v );
	}

	int Messenger::putUShort( const char* path, ushort v )
	{
		return msg_put_ushort( &msg, path, v );
	}

	int Messenger::putChar( const char* path, char v )
	{
		return msg_put_char( &msg, path, v );
	}

	int Messenger::putUChar( const char* path, uchar v )
	{
		return msg_put_uchar( &msg, path, v );
	}
}
