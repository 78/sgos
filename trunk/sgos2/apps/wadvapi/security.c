#include <sgos.h>
#include <api.h>
#include "debug.h"

struct SECURITY_DESCRIPTOR
{
	uchar Revision;
	uchar Sbz1;
	void* Control;   // point to SECURITY_DESCRIPTOR_CONTROL  typedef WORD   SECURITY_DESCRIPTOR_CONTROL
	void* Owner;  // point to PSID   typedef PVOID PSID
	void* Group;  // point to PSID   typedef PVOID PSID
	void* Sacl;   // point to ACL
	void* Dacl;      // point to ACL
};

int WINAPI InitializeSecurityDescriptor( struct SECURITY_DESCRIPTOR* p, uint revision )
{
	DBG("revision: %d", revision );
	return 0;
}

int WINAPI SetSecurityDescriptorDacl( struct SECURITY_DESCRIPTOR* p, int bDaclPresent, int pDacl, int bDaclDefaulted )
{
	NOT_IMPLEMENTED();
	return 0; //failed.
}

int WINAPI InitializeSid( void* Sid, void* pIdentifierAuthority, uchar nSubAuthorityCount )
{
	return 1; //success
	return 0; //failed.
}
