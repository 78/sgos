#include <sgos.h>
#include <debug.h>
#include <arch.h>
#include <multiboot.h>
#include <mm.h>

void kinit( uint boot_info )
{
	uint mem_size;
	multiboot_info_t* mbi = (multiboot_info_t*)boot_info;
	//init debugger as soon as possible.
	debug_init();
	
	//	check module        
	if (CHECK_FLAG (mbi->flags, 3)) 
	{ 
		module_t *mod; 
		int i; 
		kprintf ("mods_count = %d, mods_addr = 0x%x\n", 
			(int) mbi->mods_count, (int) mbi->mods_addr ); 
		for (i = 0, mod = (module_t *) mbi->mods_addr; 
			i < mbi->mods_count; i++, mod += sizeof (module_t)) 
			kprintf ("mod_start = 0x%x, mod_end = 0x%x, string = %s\n", 
				mod->mod_start, 
				mod->mod_end, 
				(char *) mod->string ); 
	} 
	// memory map
	if ( CHECK_FLAG (mbi->flags, 6) ) 
	{
		memory_map_t *mmap; 
		kprintf ("mmap_addr = 0x%x, mmap_length = 0x%x\n", 
			mbi->mmap_addr, mbi->mmap_length); 
		for ( mmap = (memory_map_t *) mbi->mmap_addr; 
				(uint) mmap < mbi->mmap_addr + mbi->mmap_length; 
				mmap = (memory_map_t *) ((uint) mmap + mmap->size + sizeof (mmap->size))
		){
			kprintf ("size = 0x%x, base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n", 
				mmap->size, 
				mmap->base_addr_high, 
				mmap->base_addr_low, 
				mmap->length_high, 
				mmap->length_low, 
				mmap->type); 
		}
	}

	// get memory size 
	if ( CHECK_FLAG (mbi->flags, 0) ) {
		mem_size = (mbi->mem_upper + 1024)<<10;
		kprintf ("mem_total = %uMB\n", mem_size>>20 );
	}
	//init machine
	machine_init();
	//page management
	page_init( mem_size );
	//init kmalloc
//	kmalloc_init();
	
	KERROR("##Warning: kernel not ready.");
}

