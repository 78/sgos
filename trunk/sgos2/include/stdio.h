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
#define	_IFMT		0170000	/* type of file */
#define		_IFDIR	0040000	/* directory */
#define		_IFCHR	0020000	/* character special */
#define		_IFBLK	0060000	/* block special */
#define		_IFREG	0100000	/* regular */
#define		_IFLNK	0120000	/* symbolic link */
#define		_IFSOCK	0140000	/* socket */
#define		_IFIFO	0010000	/* fifo */

#define 	S_BLKSIZE  1024 /* size of a block */

#define	S_ISUID		0004000	/* set user id on execution */
#define	S_ISGID		0002000	/* set group id on execution */
#ifndef	_POSIX_SOURCE
#define	S_ISVTX		0001000	/* save swapped text even after use */
#define	S_IREAD		0000400	/* read permission, owner */
#define	S_IWRITE 	0000200	/* write permission, owner */
#define	S_IEXEC		0000100	/* execute/search permission, owner */
#define	S_ENFMT 	0002000	/* enforcement-mode locking */

#define	S_IFMT		_IFMT
#define	S_IFDIR		_IFDIR
#define	S_IFCHR		_IFCHR
#define	S_IFBLK		_IFBLK
#define	S_IFREG		_IFREG
#define	S_IFLNK		_IFLNK
#define	S_IFSOCK	_IFSOCK
#define	S_IFIFO		_IFIFO
#endif	
/* !_POSIX_SOURCE */

/* The Windows header files define _S_ forms of these, so we do too
   for easier portability.  */
#define _S_IFMT		_IFMT
#define _S_IFDIR	_IFDIR
#define _S_IFCHR	_IFCHR
#define _S_IFIFO	_IFIFO
#define _S_IFREG	_IFREG
#define _S_IREAD	0000400
#define _S_IWRITE	0000200
#define _S_IEXEC	0000100

#define	S_IRWXU 	(S_IRUSR | S_IWUSR | S_IXUSR)
#define		S_IRUSR	0000400	/* read permission, owner */
#define		S_IWUSR	0000200	/* write permission, owner */
#define		S_IXUSR 0000100/* execute/search permission, owner */
#define	S_IRWXG		(S_IRGRP | S_IWGRP | S_IXGRP)
#define		S_IRGRP	0000040	/* read permission, group */
#define		S_IWGRP	0000020	/* write permission, grougroup */
#define		S_IXGRP 0000010/* execute/search permission, group */
#define	S_IRWXO		(S_IROTH | S_IWOTH | S_IXOTH)
#define		S_IROTH	0000004	/* read permission, other */
#define		S_IWOTH	0000002	/* write permission, other */
#define		S_IXOTH 0000001/* execute/search permission, other */

#define	S_ISBLK(m)	(((m)&_IFMT) == _IFBLK)
#define	S_ISCHR(m)	(((m)&_IFMT) == _IFCHR)
#define	S_ISDIR(m)	(((m)&_IFMT) == _IFDIR)
#define	S_ISFIFO(m)	(((m)&_IFMT) == _IFIFO)
#define	S_ISREG(m)	(((m)&_IFMT) == _IFREG)
#define	S_ISLNK(m)	(((m)&_IFMT) == _IFLNK)
#define	S_ISSOCK(m)	(((m)&_IFMT) == _IFSOCK)

#endif
