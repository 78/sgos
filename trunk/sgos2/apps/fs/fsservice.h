#ifndef _VFS_H_
#define _VFS_H_

#include <sgos.h>

#define BLOCK_SIZE KB(4)
#define SECTOR_SIZE 512
#define BLOCK_SIZE_BITS 12
#define BLOCK_TO_SECTOR(n) ( n * (BLOCK_SIZE/SECTOR_SIZE) )


// 文件描述符
struct DEVICE;
struct BUFFER;
typedef struct _FILE{
	char	name[FILE_NAME_LEN];
	// 文件所属设备
	struct DEVICE*	device;
	// 文件信息是否需要保存
	uchar	dirty;
	// 文件打开模式
	uint	mode;
	// 文件打开参数
	uint	flag;
	// 文件属性
	uint	attribute;
	// 是否目录
	uchar	dir;
	// 引用计数
	ushort	reference;
	// 文件大小
	size_t	size;
	// 指针位置
	size_t	pos;
	// 数据块号
	size_t	data;
	// 上一级目录
	struct _FILE	*prev, *next, *parent, *child;
	// 时间
	time_t	ctime;
	time_t	mtime;
}file_t;

// 设备描述符
typedef struct DEVICE{
	// 控制该设备的驱动名称
	char	driver[NAME_LEN];
	// 该设备服务的线程ID
	uint	thread;
	// 标识该设备的ID
	uint	devID;
	// 该设备的缓冲块链表
	struct BUFFER	*	firstBuffer;
	// 文件系统指针
	struct FILE_SYSTEM*	fs;
	// 设备文件描述符
	file_t*	devFile;
	// 设备文件系统信息
	void*	devFSInfo;
}device_t;

// 文件系统描述符
typedef struct FILE_SYSTEM{
	// 文件系统名称
	char name[NAME_LEN];
	int (*setup) (file_t* file, device_t* dev);
	int (*open) (file_t* file, const char* name);
	int (*read) (file_t* file, uchar* buf, size_t count);
	int (*write) (file_t* file, const uchar* buf, size_t count);
	int (*ioctl) (file_t* file, uint cmd, uint arg);
	int (*rename) (file_t* file, char* name);
	int (*close) (file_t* file );    //关闭文件
	int (*dir) (file_t* file, void* buf, size_t size ); //读目录项
	int (*setsize) (file_t* file, size_t size );
}fs_t;

// 虚拟文件系统服务初始化
EXTERN int vfs_init();
EXTERN int vfs_read( int id, size_t count, uchar* buf );
EXTERN int vfs_write( int id, size_t count, const uchar* buf );
EXTERN int vfs_close( int id );
EXTERN int vfs_ioctl( int id, uint cmd, uint arg );
EXTERN int vfs_open( const char* path, uint mode, uint flag );
EXTERN void funlink( file_t* f );
EXTERN int fgetid(file_t * f);
EXTERN file_t* fgetfile(int id);
EXTERN void ffree(file_t* f);
EXTERN file_t* falloc();
EXTERN void flinkto( file_t* f, file_t* parent);
extern file_t* root;

EXTERN int fs_init();
EXTERN int fs_unregister( fs_t* fs );
EXTERN int fs_register( fs_t* fs );
EXTERN fs_t* fs_detect( device_t*dev );

// 设备服务初始化
int device_init();
// 由设备名获取设备
EXTERN device_t* device_find( const char* devName );
EXTERN int device_mount( device_t* dev, const char* path );
EXTERN device_t* device_register( uint type, uint thread, uint part, char* name );

#endif
