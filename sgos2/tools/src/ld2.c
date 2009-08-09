//小虾 20090730 创建文件

#include <stdio.h>
#include <stdlib.h>
#include "bxml.h"
#include "pe.h"

static struct BXML_DATA* bxml;

static void print_help()
{
	printf("ld2 source destination\n");
}

static int translate_elf( const char* buf )
{
	return -1;	//error
}

static int translate_pe( const char* buf )
{
	IMAGE_DOS_HEADER* dos_hdr;
	IMAGE_OPTIONAL_HEADER* opt_hdr;
	FILEHDR* coff_hdr;
	unsigned int entry, base, size;
	SECHDR* sec_hdr;
	char tmp[128], *image;
	int i;
	dos_hdr = (IMAGE_DOS_HEADER*)buf;
	// 检查DOS头标记
	if( dos_hdr->e_magic != IMAGE_DOS_SIGNATURE )
		return translate_elf( buf );
	// Coff
	coff_hdr = (FILEHDR*)(buf+dos_hdr->e_lfanew+4);
	if( *(unsigned short*)(buf+dos_hdr->e_lfanew) != 0x4550 ) //'PE'
		return translate_elf( buf );
	if( !IS_EXE( coff_hdr->usFlags ) || !coff_hdr->usOptHdrSZ )
	{
		printf("not a executable pe file.");
		return -1;
	}
	opt_hdr = (IMAGE_OPTIONAL_HEADER*)((char*)coff_hdr+sizeof(*coff_hdr));
	if( opt_hdr->Magic != 0x10B )	//PE32
	{
		printf("opt_hdr=0x%X opt_hdr->Magic=0x%X\n" , (int)opt_hdr-(int)buf, opt_hdr->Magic );
		return -1;
	}
	entry = opt_hdr->AddressOfEntryPoint + opt_hdr->ImageBase;
	base = opt_hdr->ImageBase;
	size = opt_hdr->SizeOfImage;
	//模拟加载到image处，便于后面的符号操作
	image = malloc( size );
	if( !image )
		return -2;
	printf("program at address: 0x%X\t size:%d\n", base, size );
	//write bxml
	bxml_write( bxml, "program:base", &base, sizeof(base) );
	bxml_write( bxml, "program:size", &size, sizeof(size) );
	bxml_write( bxml, "program:entry_address", &entry, sizeof(entry) );
	//copy sections
	sec_hdr = (SECHDR*)((char*)opt_hdr + coff_hdr->usOptHdrSZ);
	for( i=0; i<coff_hdr->usNumSec; i++ )
	{
		sprintf( tmp, "/program/section_table/section?name=%.8s", sec_hdr[i].cName );
		bxml_redirect( bxml, tmp, 1 );
		unsigned int addr = sec_hdr[i].ulVAddr;
		bxml_write( bxml, ":virtual_address", &addr, sizeof(addr) );
		bxml_write( bxml, ":virtual_size", &sec_hdr[i].ulSize, sizeof(sec_hdr[i].ulVSize) );
		//段数据
		bxml_write( bxml, ".", buf+sec_hdr[i].ulSecOffset, sec_hdr[i].ulSize );
		memcpy( image+addr, buf+sec_hdr[i].ulSecOffset, sec_hdr[i].ulSize );
	}
	//relocations
	if( opt_hdr->NumberOfRvaAndSizes>0 && 
		opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size > 0 ){
		IMAGE_BASE_RELOCATION* rel_ent = (IMAGE_BASE_RELOCATION*)( image+
			opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].RVA );
		IMAGE_BASE_RELOCATION* rel_end = &rel_ent[
			opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size/sizeof(IMAGE_BASE_RELOCATION)];
		unsigned int *p, *nb;	//
		nb = malloc( size );
		if( !nb )
			return -1;
		//下面count_reloc计算有误，比objdump算的少了一个，大牛帮忙看看。
		p = nb;
		while( rel_ent<rel_end && rel_ent->VirtualAddress ){
			int count_reloc = (rel_ent->SizeOfBlock - 
				sizeof(IMAGE_BASE_RELOCATION)) / sizeof(unsigned short);
			unsigned short* rel_type = (unsigned short*)((unsigned int)rel_ent+
				sizeof(IMAGE_BASE_RELOCATION));
			for ( i=-1; i<count_reloc; i++ ) {
				unsigned short type = rel_type[i] >> 12;
				unsigned short value = rel_type[i] & 0x0fff;
				if ( type == IMAGE_REL_BASED_HIGHLOW ) { 
					*p = rel_ent->VirtualAddress + value;
					p++;
				}
			}
			rel_ent = (IMAGE_BASE_RELOCATION*)
				((unsigned int)rel_ent + rel_ent->SizeOfBlock);
		}
		if( (p-nb)>0 ){
			bxml_write( bxml, "/program/relocation", nb, (unsigned int)p-(unsigned int)nb );
			printf("relocation count: %d\n", (int)(p-nb) );
		}
		free( nb );
	}
	//export table
	if( opt_hdr->NumberOfRvaAndSizes>0 && 
		opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size>0 )
	{
		EXPORT_DIR_TABLE* dir_ent =  (EXPORT_DIR_TABLE*)( image+
			opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].RVA );
		//下面把Order和Name加载进来。
		char** name = (char**)(image+dir_ent->NamePointerTable);
		t_16* ordinal = (t_16*)(image+dir_ent->OrdinalTable);
		unsigned int* addr = (unsigned int*)(image+dir_ent->ExportAddressTable);
		for( i=0; i<dir_ent->NumberOfNamePointers; i++ )
		{
			sprintf( tmp, "/program/export_table/export?name=%s", image+(int)name[i] );
			bxml_redirect( bxml, tmp, 1 );
			bxml_write( bxml, ":id", &ordinal[i], sizeof(ordinal[i]) );
			bxml_write( bxml, ":virtual_address", &addr[ordinal[i]], sizeof(addr[ordinal[i]]) );
		}
	}
	//import table
	if( opt_hdr->NumberOfRvaAndSizes>0 && 
		opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size>0 )
	{
		IMPORT_DIR_TABLE* dir_ent = (IMPORT_DIR_TABLE*)( image+
			opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].RVA );
		for(; dir_ent->Name; dir_ent++ )
		{
			//some dlls;
			unsigned int* imp_ent = (unsigned int*)(image+dir_ent->ImportLookupTable);
			unsigned int imp_addr = (unsigned int)(dir_ent->ImportAddressTable);
			for(; *imp_ent; imp_ent++, imp_addr+=sizeof(imp_addr) )
			{
				if( (*imp_ent)&0x80000000 )	//import by ordinal.
				{
					printf("Sorry, import by ordinal is not supported by ld2.\n");
					return -1;
				}else{
					HINT_NAME_TABLE* hint_ent = (HINT_NAME_TABLE*)(image+(*imp_ent&0x7FFFFFFF));
					sprintf( tmp, "/program/import_table/module?name=%s/import?name=%s", 
						(image+dir_ent->Name), hint_ent->Name );
					bxml_redirect( bxml, tmp, 1 );
					bxml_write( bxml, ":id", imp_ent, sizeof(*imp_ent) );
					bxml_write( bxml, ":virtual_address", &imp_addr, sizeof(imp_addr) );
				}
			}
		}
	}
	free( image );
	return 0;
}

int main( int argc, char** argv)
{
	char* buf;
	if( argc != 3 ){
		print_help();
		return -1;
	}
	//create a bxml
	bxml = bxml_parse("<executable version=\"1.0\"></executable>");
	//work
	FILE* fp = fopen( argv[1], "rb" );
	if( fp ){
		unsigned int siz;
		fseek( fp, 0, SEEK_END );
		siz = ftell( fp );
		fseek( fp, 0, SEEK_SET );
		buf = malloc( siz );
		if( buf && fread( buf, siz, 1, fp )==1 ){
			if( translate_pe( buf ) == 0 ){
				printf("ok!\n");
				if( bxml_save( bxml, argv[2] ) < 0 ){
					printf( "error: %s\n", bxml_lasterr() );
				}
				free( buf );
				bxml_free( bxml );
				return 0;
			}
			printf("failed to translate\n");
		}
	}
	free( buf );
	bxml_free( bxml );
	printf("failed to translate file: %s\n", argv[1] );
	return -3;
}
