// Process Page Directory

#include <sgos.h>
#include <arch.h>
#include <debug.h>

// currently we support no more than 1024 processes
// running at the same time
#define MAX_PROCESS_NUM 1024
static proc_dir_bitmap[MAX_PROCESS_NUM/32] = {0,};

uint get_page_dir()
{
	register int i;
	for( i=0; i<(MAX_PROCESS_NUM>>5); i++ ){
		if( proc_dir_bitmap[i]!=0xFFFFFFFF ){
			register int j;
			for( j=0; j<32; j++ ){
				if( !(proc_dir_bitmap[i]&(1<<j)) ){
					proc_dir_bitmap[i] |= (1<<j);
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
	proc_dir_bitmap[ (addr>>5) + addr&31 ] = 0;
}
