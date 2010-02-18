#ifndef _MM_H_
#define _MM_H_

#include <sgos.h>
#include <tm.h>
#include <bigblock.h>
#include <ipc.h>

#define SPACE_MAGIC	0xFF0B46FC

#define MAX_SPACE_NUM	1024
#define MAX_SEMAPHORE_NUM	64

#define IS_KERNEL_SPACE( p ) (p->user)
#define ENV_VARIABLES_SIZE	PAGE_SIZE*4

// 物理内存使用信息
typedef struct PAGE_INFO{
	struct PAGE_INFO*	next;
	struct PAGE_INFO*	prev;
	size_t			PhysicalAddress;	//物理内存地址
	size_t			VirtualAddress;		//
	size_t			AddressInSpace0;	//
}PAGE_INFO;

// 地址空间内存信息
typedef struct KMemoryInformation{
	struct PAGE_INFO*	FirstPhysicalPage;		//使用页面链表
	uint			TotalPages;			//页面计数
	int			UserMemoryAllocated;		//用户空间分配大小
	int			KernelMemoryAllocated;		//内核空间占用大小
	bigblock_t		UserMemoryPool;		//用户空间分配管理器
	int			UserMemoryCapacity;		//用户空间允许使用内存总大小
	int			KernelMemoryCapacity;		//最大内核占用大小
}KMemoryInformation;

typedef struct KPageDirectory{
	size_t	PhysicalAddress;
	size_t	VirtualAddressInSpace0;
}KPageDirectory;

// 内核地址空间结构体。
typedef struct KSpace{
	uint				SpaceId;		//地址空间唯一ID
	uint				UserId;			//用户
	uint				Magic;			//地址空间标识
	struct KSpace*			prev, *next;		//地址空间链表，兄弟关系
	struct KSpace*			parent, *child;		//父子空间
	struct KThread*			FirstThread;		//第一个线程
	struct KThread*			MainThread;		//主线程
	struct KThread*			RealTimeThread;		//实时线程
	struct KMemoryInformation	MemoryInformation;	//内存信息
	SpaceInformation*		UserModeSpaceInformation;	//用户态信息
	KPageDirectory			PageDirectory;		//PageDirectory
	uchar				InBiosMode;		//是否使用VM86模式
} KSpace;

//第一个地址空间初始化
void MmInitializeSpaceManagement();
//返回当前地址空间
KSpace* MmGetCurrentSpace();
//创建一个地址空间
KSpace* MmCreateSpace( KSpace* parent );
int MmDestroySpace( KSpace* sp );
//由id获取地址空间的指针
KSpace* MmGetSpaceById( uint spid );
void MmInitializeSpaceBasicInformation( KSpace* space );

// kernel memory allocator
void	MmInitializeKernelMemoryPool();
void*	MmAllocateKernelMemory(size_t siz);
int	MmIsKernelMemoryAllocated(size_t addr);
void	MmFreeKernelMemory(void* p);

// user space memory allocator
void	MmInitializeUserMemoryPool(KSpace*);
void*	MmAllocateUserMemory(KSpace* space, size_t siz, uint attr, uint v);
void*	MmAllocateUserMemoryAddress(KSpace* space, size_t addr, size_t siz, uint pattr, uint v);
int	MmIsUserMemoryAllocated(KSpace*, size_t addr );
void	MmFreeUserMemory(KSpace*, void* p);
int	MmWriteUserMemory( KSpace* space, size_t addr, void* data, size_t siz );
int	MmReadUserMemory( KSpace* space, size_t addr, void* data, size_t siz );
int	MmSetUserMemoryAttribute( KSpace* space, size_t addr, size_t siz, uint attr );

//global space memory allocator
void	MmInitializeGlobalMemoryPool();
void*	MmAllocateGlobalMemory( size_t siz, uint pattr, uint v);
void	MmFreeGlobalMemory(void* p);
int	MmIsGlobalMemoryAllocated(size_t addr);

// physical
int	MmAcquireMultiplePhysicalPages( KSpace* space, size_t addr, size_t siz, uint attr, uint flag);
void	MmReleaseMultiplePhysicalPages( KSpace* space, size_t addr, size_t siz);
int	MmAcquirePhysicalPage( KSpace* space, size_t addr, uint attr, uint flag);
void	MmReleasePhysicalPage( KSpace* space, size_t addr);
void	MmDumpPhysicalPages();
void	MmFreePhysicalPage( uint addr );
uint	MmGetPhysicalPage();
void	MmInitializePhysicalMemoryManagement( size_t bitmap_start, size_t bitmap_end, size_t mem_size );
int 	MmSwapPhysicalPage( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, uint flag );
int 	MmSwapMultiplePhysicalPages( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, size_t siz, uint flag );

#endif

