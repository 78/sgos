// Memory Map
#include <sgos.h>
#include <api.h>
#include <system.h>

namespace System{
	namespace Service{
		int MapMemory( size_t vaddr, size_t paddr, size_t siz, uint flag )
		{
			return sys_vm_map( vaddr, paddr, siz, flag );
		}
		
		int UnmapMemory( size_t vaddr, size_t siz )
		{
			return sys_vm_map( vaddr, 0, 0, MAP_UNMAP );
		}
	}
}
