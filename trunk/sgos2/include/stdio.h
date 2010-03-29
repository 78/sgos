#ifndef _STDIO_H_
#define _STDIO_H_

#ifndef EXTERN
#ifdef __cplusplus 
#define EXTERN extern "C" 
#else
#define EXTERN extern
#endif
#endif

#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)

#include <types.h>
#include <string.h>
EXTERN  int printf(const char*, ... );

struct _stat {
	int	st_mode; 	//文件对应的模式，文件，目录等
	uint	st_ino; 	//inode节点号
	uint	st_dev; 	//设备号码
	uint	st_rdev; 	//特殊设备号码
	int	st_nlink; 	//文件的连接数
	int	st_uid; 	//文件所有者
	int	st_gid; 	//文件所有者对应的组
	int	st_size; 	//普通文件，对应的文件字节数
	time_t	st_atime; 	//文件最后被访问的时间
	time_t	st_mtime; 	//文件内容最后被修改的时间
	time_t	st_ctime; 	//文件状态改变时间
	size_t 	st_blksize; 	//文件内容对应的块大小
	size_t	st_blocks; 	//文件内容对应的块数量
};

#endif
