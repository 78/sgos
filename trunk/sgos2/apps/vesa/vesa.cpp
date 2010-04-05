/*
 ********************************************************************
 *                C 文件说明
 *
 * 模块：Video BIOS Extension
 * 作用：显示器驱动程序
 * 更新日期：2006-12-8 9:04
 * 作者：Huang Guan
 * 2010-4-4 移植到sgos2
 *
 **********************************************************************
 */
 
#include <sgos.h>
#include <api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vesa.h"



#define L2P(m) ( (t_32)m - mapAddress )
#define P2L(m) ( (t_32)m + mapAddress )
#define MASK_LINEAR(addr)     ((addr) & 0x000FFFFF)
#define RM_TO_LINEAR(addr)    P2L(((((addr) & 0xFFFF0000) >> 12) + ((addr) & 0xFFFF)))
#define RM_OFFSET(addr)       ((L2P(addr)) & 0xF)
#define RM_SEGMENT(addr)      (((L2P(addr)) >> 4) & 0xFFFF)


static VESA_VERSION	vesaVersion;               /* SVGA info block */
static VIDEO_MODE*	currentMode;               /* info for this video mode */
static char		oemString[256];              /* vendor name */
static ModeList		modeList = { 0, };

static ThreadContext	vmRegs = { 0, };                    /* for the bank switch routines */
static size_t		tempAddress;
static size_t		mapAddress;

//保留原来的模式
static int		oldModeNumber;
//当前显示模式
static int		curModeNumber;
//可显示区域位置
static int		curX, curY;
//？？个数，不会表达
static int		numX, numY;

static int		SetVesaMode( int mode );


static VIDEO_MODE* GetCurrentVideoMode()
{
	for( int i=0; i<modeList.total; i++ )
		if( modeList.modes[i].no == curModeNumber )
			return &modeList.modes[i];
	return 0;
}

static int GetVesaVersion()
{
	size_t addr;

	vmRegs.eax = 0x4F00;
	vmRegs.edi = RM_OFFSET((size_t)tempAddress);
	vmRegs.es = RM_SEGMENT((size_t)tempAddress);
	//DbgPrintf("VesaTemp=0x%X\nmyRegs.es=0x%X myRegs.edi=0x%X myRegs.eax=0x%X\n", VesaTemp, myRegs.es, myRegs.edi, myRegs.eax );
	VideoBiosCall(&vmRegs);
	if ((vmRegs.eax>>8))
		return -1;
	memcpy( (void*)&vesaVersion, (void*)tempAddress, sizeof(vesaVersion));

	if (strncmp((char*)vesaVersion.VESASignature, "VESA", 4) != 0)
		return -2;

	addr = RM_TO_LINEAR(vesaVersion.OEMStringPtr);

	strcpy( oemString, (char*)(addr) );
	return 0;
}

static int GetVesaModeInfo(int mode, VESA_MODE_INFO* info)
{
	vmRegs.eax = 0x4F01;
	vmRegs.edi = RM_OFFSET((t_32)tempAddress);
	vmRegs.es = RM_SEGMENT((t_32)tempAddress);
	vmRegs.ecx = mode;
	VideoBiosCall(&vmRegs);
	if((vmRegs.eax>>8))
		return -1;

	memcpy( info, (void*)tempAddress, sizeof(*info));
	return 0;
}


int SetGraphicalMode()
{
	SetVesaMode( curModeNumber );
	return 0;
}


int SetCharacterMode()
{
	vmRegs.eax = oldModeNumber;
	VideoBiosCall(&vmRegs);
	return 0;
}


static int GetVesaModeList()
{
	#define MAX_VESA_MODES 64
	t_16	mode[MAX_VESA_MODES];
	VESA_MODE_INFO info;
	t_16*	mode_ptr;
	int	c, modes, i;
	if( modeList.total > 0 ) return 0;	//该函数只调用一次。

	if (GetVesaVersion() != 0) {
		printf("[vesa]VESA not available\n");
		return -1;
	}


	mode_ptr = (t_16*)RM_TO_LINEAR(vesaVersion.VideoModePtr);
	modes = 0;

	while ((mode[modes] = *mode_ptr) != 0xFFFF) {
		modes++;
		mode_ptr++;
	}
	modeList.total = modes;
	/* search the list of modes */
	for (c=0; c<modes; c++) {
		if ( GetVesaModeInfo(mode[c], &info) == 0 ) {
			if ((info.MemoryModel == 0) || (info.BitsPerPixel < 8)) {
				modeList.total--;
				mode[c]=0;
			}
		}
	}
	//下面添加到列表中，这里申请一个4KB的页，感觉有点浪费。
	modeList.modes = (VIDEO_MODE*)malloc(sizeof(VIDEO_MODE)*modeList.total);
	i = 0;
	for (c=0; c<modes; c++) {
		if( mode[c] )
		{
			if ( GetVesaModeInfo(mode[c], &info) == 0 ) {
				modeList.modes[i].no = 0x4000|mode[c];
				modeList.modes[i].width = info.XResolution;
				modeList.modes[i].height = info.YResolution;
				modeList.modes[i].bpp = info.BitsPerPixel;
				memcpy( &modeList.modes[i].modeInfo, &info, sizeof(info) );
//				printf("Mode$ w:%d h:%d bpp:%d\n", modeList.modes[i].width,modeList.modes[i].height
//					,modeList.modes[i].bpp);
				i++;
			}
		}
	}
	return 0;
}

/* Return the current VBE video mode */
static int GetCurrentMode(void)
{
	vmRegs.eax = 0x4F03;
	VideoBiosCall(&vmRegs);
	return vmRegs.ebx;
}

static int SetVesaMode( int mode )
{
	vmRegs.eax = 0x4F02;
	vmRegs.ebx = mode;
	VideoBiosCall(&vmRegs);
	if (vmRegs.eax== 0x4F )
	{
		curModeNumber = mode;
		return 0;
	}else{
		return -1;	//error
	}
}

static int FindVideoMode( int w, int h, int bpp )
{
	int i;
	if( modeList.total < 1 )
		if( GetVesaModeList()!=0 )
			return -1;	//get mode list failed
	for( i=0; i<modeList.total; i++ )
		if( modeList.modes[i].width == w && modeList.modes[i].height == h
			&& modeList.modes[i].bpp == bpp )
			return modeList.modes[i].no;
	return -2;	//mode not found
}

int InitializeVesa()
{
	uint sid = SysGetCurrentSpaceId();
	mapAddress = (size_t)SysAllocateMemory( sid, MB(1), 0, ALLOC_VIRTUAL );
	if( mapAddress==0 ){
		printf("[vesa]failed to allocate 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	if( SysMapMemory( sid, mapAddress, MB(1), 0, 0, MAP_ADDRESS ) < 0 ){
		printf("[vesa]failed to map 1 mb memory.\n");
		return -ERR_NOMEM;
	}
	tempAddress = mapAddress + 0x00090000;
	if( GetVesaVersion() < 0 ){
		printf("[vesa] not supported.\n");
		return -ERR_INVALID;
	}
	oldModeNumber = GetCurrentMode();
	curModeNumber = oldModeNumber;
	printf("[vesa]VESA %d.%d (%s)\n", vesaVersion.VESAVersion>>8, vesaVersion.VESAVersion&0xFF, oemString );
	printf("[vesa]video memory: %d MB\n", (vesaVersion.TotalMemory<<16)/1024/1024 );
	GetVesaModeList();
	printf("[vesa]video modes: %d\n", modeList.total);
	return 0;
}

int SetVideoMode( uint w, uint h, uint bpp )
{
	int mode = FindVideoMode( w, h, bpp );
	if( mode< 0 ){
		return mode;
	}
	return SetVesaMode( mode );
}

int GetVideoMode( uint &w, uint &h, uint &bpp )
{
	VIDEO_MODE* v = GetCurrentVideoMode();
	if( v ==  0 )
		return -ERR_NONE;
	w = v->width;
	h = v->height;
	bpp = v->bpp;
	return 0;
}

int GetVideoAddress( size_t& addr, size_t & siz )
{
	VIDEO_MODE* v = GetCurrentVideoMode();
	if( v ==  0 )
		return -ERR_NONE;
	addr = v->modeInfo.PhysBasePtr;
	siz = (vesaVersion.TotalMemory<<16);
}
