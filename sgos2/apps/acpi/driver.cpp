#include <sgos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <api.h>
#include "acpi.h"
#include "port.h"

#define L2P(m) ( (size_t)(m) - mapAddress )
#define P2L(m) ( (size_t)(m) + mapAddress )

uint	*SMI_CMD;
uchar 	ACPI_ENABLE;
uchar 	ACPI_DISABLE;
uint *	PM1a_CNT;
uint *	PM1b_CNT;
ushort 	SLP_TYPa;
ushort 	SLP_TYPb;
ushort 	SLP_EN;
ushort 	SCI_EN;
uchar 	PM1_CNT_LEN;
size_t	mapAddress;

struct RSDPtr
{
	uchar	Signature[8];
	uchar	CheckSum;
	uchar	OemID[6];
	uchar	Revision;
	uint *	RsdtAddress;
};



struct FACP
{
	uchar	Signature[4];
	uint		Length;
	uchar	null1[40 - 8];
	uint *	DSDT;
	uchar 	null2[48 - 44];
	uint *	SMI_CMD;
	uchar 	ACPI_ENABLE;
	uchar 	ACPI_DISABLE;
	uchar 	null3[64 - 54];
	uint *	PM1a_CNT_BLK;
	uint *	PM1b_CNT_BLK;
	uchar 	null4[89 - 72];
	uchar 	PM1_CNT_LEN;
};


size_t CheckRsdp(size_t* ptr)
{
	const char *sig = "RSD PTR ";
	struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
	uchar *bptr;
	uchar check = 0;

	if (memcmp(sig, rsdp, 8) == 0)
	{
		// check checksum rsdpd
		bptr = (uchar *) ptr;
		for (int i=0; i<sizeof(struct RSDPtr); i++)
		{
			check += *bptr;
			bptr++;
		}

		// found valid rsdpd   
		if (check == 0) {
			if (rsdp->Revision == 0)
				printf("[acpi]Found ACPI 1 at 0x%X\n", L2P(ptr));
			else
				printf("[acpi]Found ACPI 2 at 0x%X\n", L2P(ptr));
			return (size_t) P2L(rsdp->RsdtAddress);
		}
	}

	return 0;
}

size_t GetRsdpAddress()
{
	size_t *addr;
	size_t rsdp;
	
	//Search the 0xe0000 - 0xfffff area
	for( addr = (size_t*) P2L(0xE0000); (size_t)addr < P2L(0x100000); addr += 0x10/sizeof(addr) )
	{
		rsdp = CheckRsdp( addr );
		if( rsdp )
			return rsdp;
	}
	//Serach Extended BIOS Data Area for the Root System Description Pointer signature
	//0x40:0x0E is the RM segment of the ebda
	size_t ebda = *((ushort *) P2L(0x40E));
	ebda = P2L( ebda*0x10 &0x000FFFFF );

	for (addr = (size_t *) ebda; (size_t) addr < ebda+1024; addr+= 0x10/sizeof(addr))
	{
		rsdp = CheckRsdp( addr );
		if (rsdp)
			return rsdp;
	}
	return 0;
}


// checks for a given header and validates checksum
int CheckHeader(size_t *ptr, char *sig)
{
	if (memcmp(ptr, sig, 4) == 0)
	{
		char *checkPtr = (char *) ptr;
		int len = *(ptr + 1);
		char check = 0;
		while (0<len--)
		{
			check += *checkPtr;
			checkPtr++;
		}
		if (check == 0)
			return 0;
	}
	return -1;
}


int EnableAcpi(void)
{
	// check if acpi is enabled
	if ( (inword((uint) PM1a_CNT) &SCI_EN) == 0 )
	{
		// check if acpi can be enabled
		if (SMI_CMD != 0 && ACPI_ENABLE != 0)
		{
			outbyte((uint) SMI_CMD, ACPI_ENABLE); // send acpi enable command
			// give 3 seconds time to enable acpi
			int i;
			for (i=0; i<300; i++ )
			{
				if ( (inword((uint) PM1a_CNT) &SCI_EN) == 1 )
					break;
				SysSleepThread(10);
			}
			if (PM1b_CNT != 0)
				for (; i<300; i++ )
				{
					if ( (inword((uint) PM1b_CNT) &SCI_EN) == 1 )
						break;
					SysSleepThread(10);
				}
			if (i<300) {
				printf("[acpi]enabled acpi.\n");
				return 0;
			} else {
				printf("[acpi]couldn't enable acpi.\n");
				return -1;
			}
		} else {
			printf("[acpi]no known way to enable acpi.\n");
			return -1;
		}
	} else {
		printf("[acpi]acpi was already enabled.\n");
		return 0;
	}
}



int InitializeAcpi()
{
	uint sid = SysGetCurrentSpaceId();
	mapAddress = (size_t)SysAllocateMemory( sid, MB(1), 0, ALLOC_VIRTUAL );
	if( mapAddress==0 ){
		printf("[acpi]failed to allocate 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	if( SysMapMemory( sid, mapAddress, MB(1), 0, 0, MAP_ADDRESS ) < 0 ){
		printf("[acpi]failed to map 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	
	size_t* ptr = (size_t*)GetRsdpAddress();

	// check if address is correct  ( if acpi is available on this pc )
	if ( ptr && CheckHeader( ptr, "RSDT") == 0)
	{
		// the RSDT contains an unknown number of pointers to acpi tables
		int entrys = *(ptr + 1);
		entrys = (entrys-36) /4;
		ptr += 36/4;   // skip header information

		while (0<entrys--)
		{
			// check if the desired table is reached
			if (CheckHeader((unsigned int *) P2L(*ptr), "FACP") == 0)
			{
				entrys = -2;
				struct FACP *facp = (struct FACP *) P2L(*ptr);
				if (CheckHeader((unsigned int *) P2L(facp->DSDT), "DSDT") == 0)
				{
					// search the \_S5 package in the DSDT
					char *S5Addr = (char *) P2L(facp->DSDT) +36; // skip header
					int dsdtLength = *(size_t*)(P2L(facp->DSDT)+4) -36;
					while (0 < dsdtLength--)
					{
						if ( memcmp(S5Addr, "_S5_", 4) == 0)
							break;
						S5Addr++;
					}
					// check if \_S5 was found
					if (dsdtLength > 0)
					{
						// check for valid AML structure
						if ( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 )
						{
							S5Addr += 5;
							S5Addr += ((*S5Addr &0xC0)>>6) +2;   // calculate PkgLength size

							if (*S5Addr == 0x0A)
								S5Addr++;   // skip byteprefix
							SLP_TYPa = *(S5Addr)<<10;
							S5Addr++;

							if (*S5Addr == 0x0A)
								S5Addr++;   // skip byteprefix
							SLP_TYPb = *(S5Addr)<<10;

							SMI_CMD = facp->SMI_CMD;

							ACPI_ENABLE = facp->ACPI_ENABLE;
							ACPI_DISABLE = facp->ACPI_DISABLE;

							PM1a_CNT = facp->PM1a_CNT_BLK;
							PM1b_CNT = facp->PM1b_CNT_BLK;

							PM1_CNT_LEN = facp->PM1_CNT_LEN;

							SLP_EN = 1<<13;
							SCI_EN = 1;
							return 0;
						} else {
							printf("[acpi]\\_S5 parse error.\n");
						}
					} else {
						printf("[acpi]\\_S5 not present.\n");
					}
				} else {
					printf("[acpi]DSDT invalid.\n");
				}
			}
			ptr++;
		}
		printf("[acpi]no valid FACP present.\n");
	} else {
		printf("[acpi]no acpi found.\n");
	}
	return -1;
}


void AcpiPowerOff(void)
{
   // SCI_EN is set to 1 if acpi shutdown is possible
   if (SCI_EN == 0)
      return;

   EnableAcpi();

   // send the shutdown command
   outword((unsigned int) PM1a_CNT, SLP_TYPa | SLP_EN );
   if ( PM1b_CNT != 0 )
      outword((unsigned int) PM1b_CNT, SLP_TYPb | SLP_EN );

   printf("[acpi]acpi poweroff failed.\n");
}
