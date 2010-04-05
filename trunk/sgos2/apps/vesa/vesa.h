#ifndef _VESA_H_
#define _VESA_H_

#include <sgos.h>


typedef struct DisplayMode{
	int width;
	int height;
	int bpp;
}DisplayMode;

//下面的几个结构来自VESA官方文档?
#define __PACKED__   __attribute__ ((packed))
typedef struct tagVesaInfo         /* VESA information block structure */
{
	uchar  VESASignature[4]     __PACKED__;	//Must be VESA
	ushort VESAVersion          __PACKED__;
	ulong  OEMStringPtr         __PACKED__;
	uchar  Capabilities[4]      __PACKED__;
	ulong  VideoModePtr         __PACKED__;
	ushort TotalMemory          __PACKED__;
	ushort OemSoftwareRev       __PACKED__;
	ulong  OemVendorNamePtr     __PACKED__;
	ulong  OemProductNamePtr    __PACKED__;
	ulong  OemProductRevPtr     __PACKED__;
	uchar  Reserved[222]        __PACKED__;
	uchar  OemData[256]         __PACKED__;
}VESA_VERSION;

typedef struct _ModeInfo         /* VESA information for a specific mode */
{
	ushort ModeAttributes       __PACKED__;
	uchar  WinAAttributes       __PACKED__;
	uchar  WinBAttributes       __PACKED__;
	ushort WinGranularity       __PACKED__;
	ushort WinSize              __PACKED__;
	ushort WinASegment          __PACKED__;
	ushort WinBSegment          __PACKED__;
	ulong  WinFuncPtr           __PACKED__;
	ushort BytesPerScanLine     __PACKED__;
	ushort XResolution          __PACKED__;
	ushort YResolution          __PACKED__;
	uchar  XCharSize            __PACKED__;
	uchar  YCharSize            __PACKED__;
	uchar  NumberOfPlanes       __PACKED__;
	uchar  BitsPerPixel         __PACKED__;
	uchar  NumberOfBanks        __PACKED__;
	uchar  MemoryModel          __PACKED__;
	uchar  BankSize             __PACKED__;
	uchar  NumberOfImagePages   __PACKED__;
	uchar  Reserved_page        __PACKED__;
	uchar  RedMaskSize          __PACKED__;
	uchar  RedMaskPos           __PACKED__;
	uchar  GreenMaskSize        __PACKED__;
	uchar  GreenMaskPos         __PACKED__;
	uchar  BlueMaskSize         __PACKED__;
	uchar  BlueMaskPos          __PACKED__;
	uchar  ReservedMaskSize     __PACKED__;
	uchar  ReservedMaskPos      __PACKED__;
	uchar  DirectColorModeInfo  __PACKED__;

	/* VBE 2.0 extensions */
	ulong  PhysBasePtr          __PACKED__;
	ulong  OffScreenMemOffset   __PACKED__;
	ushort OffScreenMemSize     __PACKED__;
	uchar  LinNumberOfPages     __PACKED__;
	uchar  LinRedMaskSize       __PACKED__;
	uchar  LinRedFieldPos       __PACKED__;

	/* VBE 3.0 extensions */
	ushort LinBytesPerScanLine  __PACKED__;
	uchar  BnkNumberOfPages     __PACKED__;
	uchar  LinGreenMaskSize     __PACKED__;
	uchar  LinGreenFieldPos     __PACKED__;
	uchar  LinBlueMaskSize      __PACKED__;
	uchar  LinBlueFieldPos      __PACKED__;
	uchar  LinRsvdMaskSize      __PACKED__;
	uchar  LinRsvdFieldPos      __PACKED__;
	ulong  MaxPixelClock        __PACKED__;

	uchar  Reserved[190]        __PACKED__;
}VESA_MODE_INFO;

//显示模式
typedef struct tagVideoMode{
	ushort	width;
	ushort	height;
	uchar	bpp;
	ushort	no;
	VESA_MODE_INFO modeInfo;
}VIDEO_MODE;

typedef struct tagModeList{
	int		total;
	VIDEO_MODE	*modes;
}ModeList;


int SetGraphicalMode();
int SetCharacterMode();
int InitializeVesa();
int SetVideoMode( uint w, uint h, uint bpp );
int GetVideoMode( uint &w, uint &h, uint &bpp );
int GetVideoAddress( size_t& addr, size_t & siz );
int VideoBiosCall( ThreadContext* context );

#endif
