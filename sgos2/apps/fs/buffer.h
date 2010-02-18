#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <sgos.h>
#include <queue.h>
#include "fsservice.h"

// 缓冲区块结构
typedef struct BUFFER{
	struct BUFFER	*next, *prev;
	// 指向所属设备
	device_t	*device;
	// 指向块号
	uint		block;
	// 块数据
	uchar		*data;
	// 是否修改过
	uchar		dirty;
	// 引用计数
	ushort		reference;
	// 创建时间，依次判断该缓冲区块是否抛弃
	time_t		ctime;
}buffer_t;

// 磁盘缓冲区初始化
EXTERN int		buffer_init( size_t buf_mem );
// 获取缓冲区
EXTERN buffer_t*	buffer_get( device_t* dev, size_t block );
// 释放缓冲区
EXTERN void		buffer_put( buffer_t* buf );
// 写入所有修改过的缓冲区
EXTERN void		buffer_sync();

#endif
