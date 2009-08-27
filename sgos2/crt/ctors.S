.global _get_ctors
.global _get_dtors
_get_ctors:
	lea __CTOR_LIST__, %eax
	ret

_get_dtors:
	lea __DTOR_LIST__, %eax
	ret
