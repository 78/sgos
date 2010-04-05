#ifndef _MM_H_
#define _MM_H_

#include <sgos.h>
#include <tm.h>
#include <ipc.h>

#define SPACE_MAGIC	0xFF0B46FC

#define MAX_SPACE_NUM	1024
#define MAX_SEMAPHORE_NUM	64

#define IS_KERNEL_SPACE( p ) (p->user)
#define ENV_VARIABLES_SIZE	PAGE_SIZE*4

// 虚拟内存使用信息
typedef struct KVirtualMemoryAllocation{
	struct KVirtualMemoryAllocation*	next;
	struct KVirtualMemoryAllocation*	prev;
	size_t			VirtualAddress;
	size_t			VirtualSize;
	uint			MemoryAttribute;
	uint			AllocationFlag;
}KVirtualMemoryAllocation;

typedef struct KVirtualMemory{
	KVirtualMemoryAllocation*	FirstAllocation;		//使用页面链表
	KVirtualMemoryAllocation*	LastAllocation;		//使用页面链表
	size_t				LowerLimit;
	size_t				UpperLimit;
	KSemaphore			AccessLock;
}KVirtualMemory;

// 地址空间内存信息
typedef struct KMemoryInformation{
	int				UserMemoryAllocated;		//用户空间分配大小
	int				KernelMemoryAllocated;		//内核空间占用大小
	int				UserMemoryCapacity;		//用户空间允许使用内存总大小
	int				KernelMemoryCapacity;		//最大内核占用大小
}KMemoryInformation;

typedef struct KPageDirectory{
	size_t	PhysicalAddress;
	size_t	VirtualAddress;
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
	struct KVirtualMemory		VirtualMemory;
	SpaceInformation*		UserModeSpaceInformation;	//用户态信息
	KPageDirectory			PageDirectory;		//PageDirectory
	uchar				InBiosMode;		//是否使用VM86模式
} KSpace;

void 	MmInitializeSpaceManagement();
KSpace* MmGetCurrentSpace();
KSpace* MmCreateSpace( KSpace* parent );
int 	MmDestroySpace( KSpace* sp );
KSpace* MmGetSpaceById( uint spid );
void 	MmInitializeSpaceBasicInformation( KSpace* space );

// kernel memory allocator
void	MmInitializeKernelMemoryPool();
void*	MmAllocateKernelMemory(size_t siz);
int	MmIsKernelMemoryAllocated(size_t addr);
void	MmFreeKernelMemory(void* p);
extern 	KVirtualMemory KernelVirtualMemory;

// virtual memory allocator
void 	MmInitializeVirtualMemory( KVirtualMemory *vm, size_t beg, size_t end );
void* 	MmAllocateVirtualMemory( KVirtualMemory* vm, size_t addr, size_t size, uint attr, uint flag );
size_t  MmFreeVirtualMemory( KVirtualMemory* vm, void* p );
KVirtualMemoryAllocation* 
	MmGetVmaByAddress( KVirtualMemory* vm, size_t addr );
int 	MmQueryVirtualAddressInformation( KVirtualMemory* vm, size_t addr, size_t* beg, size_t* end, uint *attr, uint *flag );

// user space memory
void*	MmAllocateUserMemory(KSpace* space, size_t siz, uint attr, uint v);
void*	MmAllocateUserMemoryAddress(KSpace* space, size_t addr, size_t siz, uint pattr, uint v);
int	MmIsUserMemoryAllocated(KSpace*, size_t addr );
void	MmFreeUserMemory(KSpace*, void* p);

//global space memory allocator
void	MmInitializeGlobalMemoryPool();
void*	MmAllocateGlobalMemory( size_t siz, uint pattr, uint v);
void	MmFreeGlobalMemory(void* p);
extern 	KVirtualMemory GlobalVirtualMemory;

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
int		MmHandleNonPresentPageFault( KSpace* space, size_t addr );
int		MmHandleReadOnlyPageFault( KSpace* space, size_t addr );
int 	MmMapMemory( KSpace* space, uint virt_addr, uint phys_addr, uint size, uint attr, uint flag );
int 	MmDuplicateMultiplePhysicalPages( KSpace* destSp, size_t dest_addr, KSpace* srcSp, size_t src_addr, size_t siz );

#endif

