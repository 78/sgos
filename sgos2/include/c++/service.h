#ifndef _SERVICE_H_

namespace Service{
	//IO操作
	uchar inbyte( ushort port );
	ushort inword( ushort port );
	uint indword( uint port );
	void outbyte( ushort port, uchar v );
	void outword( ushort port, ushort v );
	void outdword( ushort port, uint v );
	int MapMemory( size_t vaddr, size_t paddr, size_t siz, uint flag );
	int UnmapMemory( size_t vaddr, size_t siz );
	int CallBIOS( int interrupt, void* tc );
	int CallBIOS( int interrupt, int& eax, int& ebx, int& ecx, int& edx );
};

#endif

