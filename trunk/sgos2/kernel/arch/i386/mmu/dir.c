// Page Directory

#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <rtl.h>

// currently we support no more than 1024 spaces
// running at the same time
#define MAX_SPACE_NUM 1024
static size_t dir_bitmap[MAX_SPACE_NUM/32] = {0,};

//页目录初始化
void ArInitializePageDirectoryManagement()
{
	RtlZeroMemory32( dir_bitmap, (sizeof(size_t)*MAX_SPACE_NUM)>>(5+2) );
}

//获取一个给进页目录，返回的是虚拟地址
size_t ArAllocatePageDirecotry()
{
	register int i;
	size_t eflags;
	ArLocalSaveIrq( eflags );
	for( i=0; i<(MAX_SPACE_NUM>>5); i++ ){
		if( dir_bitmap[i]!=0xFFFFFFFF ){
			register int j;
			for( j=0; j<32; j++ ){
				if( !(dir_bitmap[i]&(1<<j)) ){
					dir_bitmap[i] |= (1<<j);
					ArLocalRestoreIrq( eflags );
					return ALLSPACES_PAGEDIR_BEG+ (((i<<5)+j)<<PAGE_SIZE_BITS);
				}
			}
		}
	}
	KERROR("## no page dir. Should do something here ...");
	return 0;
}

//释放一个页目录
void ArFreePageDirectory(size_t addr)
{
	addr -= ALLSPACES_PAGEDIR_BEG;
	addr >>= PAGE_SIZE_BITS;
	dir_bitmap[addr>>5] &= ~(1<<(addr&31));
}




