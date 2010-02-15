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

#endif

