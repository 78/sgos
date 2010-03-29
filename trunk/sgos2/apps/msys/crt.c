#include <stdlib.h>
#include "cygwin.h"
#include "debug.h"


typedef int ResourceLocks ;
typedef int MTinterface ;
ResourceLocks _reslock ;
MTinterface _mtinterf;
void* reent_data;
/* Pointer into application's static data */
struct per_process __cygwin_user_data =
{/* initial_sp */ 0, /* magic_biscuit */ 0,
/* dll_major */ 1,
/* dll_major */ 1,
/* impure_ptr_ptr */ NULL, /* envptr */ NULL,
/* malloc */ (void*)malloc, /* free */ (void*)free,
/* realloc */ (void*)realloc,
/* fmode_ptr */ NULL, /* main */ NULL, /* ctors */ NULL,
/* dtors */ NULL, /* data_start */ NULL, /* data_end */ NULL,
/* bss_start */ NULL, /* bss_end */ NULL,
/* calloc */ (void*)calloc,
/* premain */ {NULL, NULL, NULL, NULL},
/* run_ctors_p */ 0,
/* unused */ {0, 0, 0, 0, 0, 0, 0},
/* forkee */ 0,
/* hmodule */ 0,
/* api_major */ 2,
/* api_minor */ 2,
/* unused2 */ {0, 0, 0, 0, 0},
/* resourcelocks */ &_reslock, /* threadinterface */ &_mtinterf,
/* impure_ptr */ (void*)&reent_data,
};
#define user_data (&__cygwin_user_data)

void __main (void)
{
}

uint cygwin_internal( int type )
{
	DBG("type=%d", type );
	switch( type ){
	case CW_USER_DATA:
		memset( user_data, 0, sizeof( __cygwin_user_data ) );
		return (uint)user_data;
	}
}

void dll_crt0__FP11per_process()
{
	NOT_IMPLEMENTED();
}

