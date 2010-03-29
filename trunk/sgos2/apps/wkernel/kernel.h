#ifndef _HANDLE_

#define STD_INPUT_HANDLE	-10
#define STD_OUTPUT_HANDLE	-11
#define STD_ERROR_HANDLE	-12


typedef struct _STARTUPINFO {
  uint  cb;
  char* lpReserved;
  char* lpDesktop;
  char* lpTitle;
  uint  dwX;
  uint  dwY;
  uint  dwXSize;
  uint  dwYSize;
  uint  dwXCountChars;
  uint  dwYCountChars;
  uint  dwFillAttribute;
  uint  dwFlags;
  ushort   wShowWindow;
  ushort   cbReserved2;
  uchar* lpReserved2;
  uint hStdInput;
  uint hStdOutput;
  uint hStdError;
} STARTUPINFO, *LPSTARTUPINFO;

enum HandleType{
	TYPE_UNKNOWN = 0,
	TYPE_DISK = 0x0001,
	TYPE_CHAR = 0x0002,
	TYPE_PIPE = 0x0003,
	TYPE_MODULE = 0x0004,
	TYPE_FILE,
	TYPE_PROCESS,
	TYPE_THREAD,
	TYPE_EVENT
};

uint _CreateHandle( int type, uint data );
void _CloseHandle( uint h );
uint _GetHandleData( uint h );
uint _GetHandleType(uint h );

#endif
