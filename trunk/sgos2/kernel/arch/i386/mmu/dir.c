// Process Page Directory

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>

// currently we support no more than 1024 processes
// running at the same time
#define MAX_PROCESS_NUM 1024
static uint proc_dir_bitmap[MAX_PROCESS_NUM/32] = {0,};

void dir_init()
{
	memsetd( proc_dir_bitmap, 0, (sizeof(uint)*MAX_PROCESS_NUM)>>(5+2) );
}

uint get_page_dir()
{
	register int i;
	uint eflags;
	local_irq_save( eflags );
	for( i=0; i<(MAX_PROCESS_NUM>>5); i++ ){
		if( proc_dir_bitmap[i]!=0xFFFFFFFF ){
			register int j;
			for( j=0; j<32; j++ ){
				if( !(proc_dir_bitmap[i]&(1<<j)) ){
					proc_dir_bitmap[i] |= (1<<j);
					local_irq_restore( eflags );
					return PROC_PAGE_DIR_BASE+ (((i<<5)+j)<<PAGE_SIZE_BITS);
				}
			}
		}
	}
	KERROR("no page dir.");
	return 0;
}

void free_page_dir(uint addr)
{
	addr -= PROC_PAGE_DIR_BASE;
	addr >>= PAGE_SIZE_BITS;
	proc_dir_bitmap[addr>>5] &= ~(1<<(addr&31));
}




