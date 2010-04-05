#include <sgos.h>
#include <api.h>
#include "vesa.h"


int VideoBiosCall( ThreadContext* context )
{
	return Api_InvokeBiosService( 0x10, (void*)context, sizeof(*context) );
}
