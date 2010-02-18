//user space memory bigblock allocator
//provide a lot of memory for running SGOS2 user process
//use 0x00000000 - 0x80000000  2GB  VIRTUAL_MEMORY

#include <sgos.h>
#include <bigblock.h>
#include <kd.h>
#include <mm.h>

// 用户空间内存初始化
void MmInitializeUserMemoryPool( KSpace* space )
{
	bb_init_block( &space->MemoryInformation.UserMemoryPool, 
		0x00100000,		//address
		0x7FF00000		//size
		);
	//
}

// 
static int	ReadWriteUserMemory( KSpace* space, size_t addr, void* buf, size_t siz, int write)
{
	int remain = siz, ret, bytes;
	void* p;
	size_t page, pos = addr, phys_addr, attr;
	KSpace* currentSpace = MmGetCurrentSpace();
	if( currentSpace == space ){
		if(write)
			RtlCopyMemory( (void*)addr, buf, siz );
		else
			RtlCopyMemory( buf, (void*)addr, siz );
		return siz;
	}
	p = MmAllocateUserMemory(currentSpace, PAGE_SIZE, 0, ALLOC_VIRTUAL);
	if( p==NULL ){
		PERROR("Failed to allocate currentSpace user memory");
		return 0;
	}
	//一页一页地映射，写进去，为了保证不发生异常，是否应该检查页属性呢
	while( remain > 0 ){
		page = ( pos >> PAGE_SIZE_BITS ) << PAGE_SIZE_BITS;
		bytes = (page==pos)?PAGE_SIZE : PAGE_SIZE-(pos-page);
		if( bytes>remain )
			bytes = remain;
		ret = ArQueryPageInformation( &space->PageDirectory, page, &phys_addr, &attr );
		if(  ret==0 && (attr&PAGE_ATTR_ALLOCATED) && (attr&PAGE_ATTR_USER) && (attr&PAGE_ATTR_PRESENT) &&
			!(write && !(attr&PAGE_ATTR_WRITE)) //保证页面可以写入
			){
			ArMapOnePage( &currentSpace->PageDirectory, (size_t)p, phys_addr, attr
				, MAP_ATTRIBUTE|MAP_ADDRESS );
			if( write )
				RtlCopyMemory( (void*)((size_t)p+pos-page), buf, bytes );
			else
				RtlCopyMemory( buf, (void*)((size_t)p+pos-page), bytes );
			ArMapOnePage( &currentSpace->PageDirectory, (size_t)p, 0, 0
				, MAP_ATTRIBUTE );
			buf = (void*)((size_t)buf + bytes);
			pos += bytes;
			remain -= bytes;
		}else{ //failed
			PERROR("##failed at %x", p);
			MmFreeUserMemory(currentSpace, p);
			return siz-remain;
		}
	}
	MmFreeUserMemory(currentSpace, p);
	return siz;
}

// 写入数据到指定地址空间
int	MmWriteUserMemory( KSpace* space, size_t addr, void* data, size_t siz )
{
	return ReadWriteUserMemory( space, addr, data, siz, 1 );
}

// 从指定地址空间读取数据
int	MmReadUserMemory( KSpace* space, size_t addr, void* data, size_t siz )
{
	return ReadWriteUserMemory( space, addr, data, siz, 0 );
}

// 设置指定空间的内存页面属性
int	MmSetUserMemoryAttribute( KSpace* space, size_t addr, size_t siz, uint attr )
{
	ASSERT( addr%PAGE_SIZE==0 && siz%PAGE_SIZE==0 );
	ArMapMultiplePages( &space->PageDirectory, addr, siz, 0, 
		attr|PAGE_ATTR_USER|PAGE_ATTR_ALLOCATED, MAP_ATTRIBUTE );
	return 0;
}

// 分配指定地址的用户态空间内存
void*	MmAllocateUserMemoryAddress(KSpace* space, size_t addr, size_t siz, uint pattr, uint flag)
{
	KMemoryInformation* info;
	void *ptr;
	int ret;
	if( !space )
		return NULL;
	ASSERT( addr%PAGE_SIZE==0 && siz%PAGE_SIZE==0 );
	//change some mem info
	info = &space->MemoryInformation;
	info->UserMemoryAllocated += siz;
	if( info->UserMemoryAllocated > info->UserMemoryCapacity ){
		info->UserMemoryAllocated -= siz;
		PERROR("##user memory used out!");
		return NULL;
	}
	if( addr == 0 ){
		ptr = bb_alloc( &info->UserMemoryPool, siz );
	}else{
		ptr = bb_alloc_ex( &info->UserMemoryPool, addr, siz );
	}
	if( ptr==NULL ){
		//no memory?? 分配失败就撤销增加
		info->UserMemoryAllocated -= siz;
		return NULL;
	}
	if( !(flag&ALLOC_VIRTUAL) ){ //是否立刻分配物理地址
		uint mapFlag = MAP_ATTRIBUTE;
		if( flag&ALLOC_ZERO )
			mapFlag |= MAP_ZERO;
		ret = MmAcquireMultiplePhysicalPages( space, (size_t)ptr, 
			siz, pattr|PAGE_ATTR_USER|PAGE_ATTR_ALLOCATED, mapFlag );
		if( ret<0 ){ 
			MmFreeUserMemory(space, ptr);
			return NULL;
		}
	}else{
		ArMapMultiplePages( &space->PageDirectory, (size_t)ptr, 0, siz,
			PAGE_ATTR_USER|PAGE_ATTR_ALLOCATED|pattr, MAP_ATTRIBUTE );
	}
	return ptr;
}

// 分配用户态空间的内存
void*	MmAllocateUserMemory(KSpace* space, size_t siz, uint attr, uint flag)
{
	return MmAllocateUserMemoryAddress( space, 0, siz, attr, flag );
}

// 释放用户空间的内存
void	MmFreeUserMemory(KSpace* space, void* p)
{
	KMemoryInformation* info;
	size_t siz;
	uint state;
	if( space ){
		//info之前忘了赋值。。。
		//弄了乱出八糟的错误，调试了一天。。。
		info = &space->MemoryInformation;
		//此时避免其他线程使用内存分配。
		TmSaveScheduleState(state);
		siz = bb_free( &info->UserMemoryPool, p );
		//释放物理地址
		MmReleaseMultiplePhysicalPages( space, (size_t)p, siz );
		TmRestoreScheduleState(state);
		//change some mem info
		info->UserMemoryAllocated -= siz;
		if( info->UserMemoryAllocated < 0 )
			info->UserMemoryAllocated = 0;
	}
}

//判断内存地址是否已分配
int	MmIsUserMemoryAllocated(KSpace* space, size_t addr)
{
	KMemoryInformation* info;
	if( space ){
		//change some mem info
		info = &space->MemoryInformation;
		return bb_check_allocated( &info->UserMemoryPool, addr );
	}
	return 0;
}
