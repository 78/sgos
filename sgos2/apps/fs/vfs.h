#ifndef _VFS_H_
#define _VFS_H_

#include <sgos.h>
#include <queue.h>

#define BLOCK_SIZE 1024
#define BLOCK_SIZE_BITS 10
#define BLOCK_TO_SECTOR(n) (n<<1)


// 文件描述符
struct DEVICE;
typedef struct _FILE{
	char	name[FILE_NAME_LEN];
	// 文件的绝对路径
	char	path[PATH_LEN];
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
	struct _FILE*	parent;
	// 时间
	time_t	ctime;
	time_t	mtime;
}file_t;

// 设备描述符
typedef struct DEVICE{
	// 控制该设备的驱动名称
	char	driver[NAME_LEN];
	// 标识该设备的ID
	uint	devID;
	// 挂载该设备的名称
	char	name[NAME_LEN];
	// 该设备的缓冲块队列
	queue_t	bufferQueue;
	// 文件系统指针
	struct FILE_SYSTEM*	fs;
	// 设备文件描述符
	file_t	devFile;
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

#define MAX_OPEN_FILE 512

// 进程打开文件描述符
typedef struct PROCESS_OPENLIST{
	uint process;
	file_t*	openList[MAX_OPEN_FILE];
}procinfo_t;

// 虚拟文件系统服务初始化
int vfs_init();
EXTERN int vfs_read( uint proc, int id, size_t count, uchar* buf );
EXTERN int vfs_write( uint proc, int id, size_t count, const uchar* buf );
EXTERN int vfs_close( uint proc, int id );
EXTERN int vfs_ioctl( uint proc, int id, uint cmd, uint arg );
EXTERN int vfs_open( uint proc, const char* path, uint mode, uint flag );

int fs_init();
EXTERN int fs_unregister( fs_t* fs );
EXTERN int fs_register( fs_t* fs );
EXTERN fs_t* fs_detect( device_t*dev );

// 设备服务初始化
int device_init();
// 由设备名获取设备
EXTERN device_t* device_find( const char* devName );
EXTERN int device_mount( device_t* dev, const char* path );
EXTERN device_t* device_register( const char* driver, uint part );

#endif
