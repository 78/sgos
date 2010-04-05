// Micro Kernel
#ifndef _KE_H
#define _KE_H

#include <sgos.h>

int KeStartSystemThread();
int KeCreateBaseService();
int KeLoadPeExecutable();
int KeCreateRemoteThread();

void KeStartOs( size_t boot_info );
void KeBugCheck(const char *s );
void KeResumeStart();

void KeInitializeSystemInformation();
SystemInformation* KeGetSystemInforamtion();

int KeHardwareInterruptMessage( int no );
void KeDeleteHardwareInterruptHandler( int no, uint threadId );
int KeAddHardwareInterruptHandler( int no, uint threadId );
void KeEnableHardwareInterrupt( int no, int b );
void KeInitializeHardwareIntterupt();

#endif

