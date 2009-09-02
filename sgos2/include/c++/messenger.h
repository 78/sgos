#ifndef _MESSENGER_H_
#define _MESSENGER_H_

#include <sgos.h>
#include <msg.h>
// 临时对msg进行封装，以后可以改进，直接使用系统调用。
class Messenger{
private:
	messenger_t msg;
	uint flag;	//send/recv flag
public:
	Messenger();
	Messenger( const char* str );
	~Messenger();
	//useful functions
	// 得到一个回复用的Messenger
	Messenger	reply();
	// 获取发送者信息
	uint	getSenderThread();
	uint	getSenderProcess();
	// 添加一个兄弟节点
	Messenger&	append( const char* name );
	// 解析一个消息文本
	int	parse( const char* str );
	// 创建一个路径，并且转到创建的路径
	int	mkdir( const char* path );
	// 重定位当前路径
	int	redir( const char* path );
	// 移动到下一项
	int	moveNext();
	// 提交一个同步请求
	int	request();
	// 发送消息，异步
	int	send();
	// 接收消息
	int	receive(bool pending = true);
	// 读操作
	int 	read( const char* path, void* buf, uint buf_size );
	// 读节点名
	const char* readName( const char* path );
	// 写操作
	int 	write( const char* path, const void* buf, uint buf_size );
	// 读取一个字符串
	char*	getString( const char* path );
	int	getInt( const char* path );
	uint	getUInt( const char* path );
	short	getShort( const char* path );
	ushort	getUShort( const char* path );
	char	getChar( const char* path );
	uchar	getUChar( const char* path );
	uchar	getByte( const char* path );
	// 写入一个字符串
	int	putString( const char* path, const char* str );
	int	putInt( const char* path, int v );
	int	putUInt( const char* path, uint v );
	int	putShort( const char* path, short v );
	int	putUShort( const char* path, ushort v );
	int	putChar( const char* path, char v );
	int	putUChar( const char* path, uchar v );
};

#endif
