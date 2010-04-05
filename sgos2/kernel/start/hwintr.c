#include <sgos.h>
#include <arch.h>
#include <kd.h>
#include <ke.h>
#include <rtl.h>
#include <ipc.h>

#define MAX_INTERRUPT			32
#define MAX_HANDLER_PER_INTERRUPT	4

static uint handlers[MAX_INTERRUPT][MAX_HANDLER_PER_INTERRUPT];

void KeInitializeHardwareIntterupt()
{
	RtlZeroMemory( handlers, sizeof(handlers) );
}

void KeEnableHardwareInterrupt( int no, int b )
{
	ArSetIrqMask( no, b );
}

int KeAddHardwareInterruptHandler( int no, uint threadId )
{
	int i;
	for( i=0; i<MAX_HANDLER_PER_INTERRUPT; i++ ){
		if( handlers[no][i] == threadId )
			return 0;
		if( handlers[no][i] == 0 ){
			handlers[no][i] = threadId;
			return 0;
		}
	}
	PERROR("Failed to add the handler %d to thread:%x.", no, threadId );
	return -1;
}

void KeDeleteHardwareInterruptHandler( int no, uint threadId )
{
	int i;
	for( i=0; i<MAX_HANDLER_PER_INTERRUPT; i++ ){
		if( handlers[no][i] == threadId ){
			handlers[no][i] = 0;
			return;
		}
	}
}

int KeHardwareInterruptMessage( int no )
{
	int i, ret;
	for( i=0; i<MAX_HANDLER_PER_INTERRUPT; i++ ){
		if( handlers[no][i] )
		{
			ArSetIrqMask( no, 0 );
			ret = IpcQuickSend( handlers[no][i], System_Interrupt, no, 0 );
			if( ret < 0 ){
				PERROR("Failed to send interrupt message to thread: 0x%X", handlers[no][i] );
				handlers[no][i] = 0;
			}
		}
	}
	return 0;
}


