// 
#include <sgos.h>
#include <api.h>
#include <string.h>
#include <system.h>

namespace System{
	namespace Service{
		int CallBIOS( int interrupt, int& eax, int& ebx, int& ecx, int& edx )
		{
			THREAD_CONTEXT tc;
			int ret;
			memset( &tc, 0, sizeof(THREAD_CONTEXT) );
			tc.eax = eax;
			tc.ebx = ebx;
			tc.ecx = ecx;
			tc.edx = edx;
			ret = sys_bios_call( interrupt, &tc, sizeof(THREAD_CONTEXT) );
			eax = tc.eax;
			ebx = tc.ebx;
			ecx = tc.ecx;
			edx = tc.edx;
			return ret;
		}
		
		int CallBIOS( int interrupt, void* tc )
		{
			return sys_bios_call( interrupt, tc, sizeof(THREAD_CONTEXT) );
		}
	}
}
