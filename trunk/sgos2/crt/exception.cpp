#define PERSONALITY_FUNCTION __gxx_personality_sj0

typedef char _Unwind_Exception_Class[8];

typedef enum
{
	_URC_OK = 0,       /* operation completed successfully */
	_URC_FOREIGN_EXCEPTION_CAUGHT = 1,
	_URC_END_OF_STACK = 5,
	_URC_HANDLER_FOUND = 6,
	_URC_INSTALL_CONTEXT = 7,
	_URC_CONTINUE_UNWIND = 8,
	_URC_FAILURE = 9   /* unspecified failure of some kind */
}_Unwind_Reason_Code;

typedef enum
{
	_US_VIRTUAL_UNWIND_FRAME = 0,
	_US_UNWIND_FRAME_STARTING = 1,
	_US_UNWIND_FRAME_RESUME = 2,
	_US_ACTION_MASK = 3,
	_US_FORCE_UNWIND = 8,
	_US_END_OF_STACK = 16
}_Unwind_State;

struct _Unwind_Exception
{
    unsigned exception_class __attribute__((__mode__(__DI__)));
    void * exception_cleanup;
    unsigned private_1 __attribute__((__mode__(__word__)));
    unsigned private_2 __attribute__((__mode__(__word__)));
} __attribute__((__aligned__));

struct _Unwind_Context {
	unsigned	entry_regs[16];
	unsigned	current_regs[16];
	unsigned	cfa;
	unsigned	pc;
	unsigned	ra;
	void		*fde;
	void*  pfn;
	unsigned	func;
	void		*lsda;
	unsigned	range;
};

/* Provided only for for compatibility with existing code.  */
typedef int _Unwind_Action;
#define _UA_SEARCH_PHASE        1
#define _UA_CLEANUP_PHASE       2
#define _UA_HANDLER_FRAME       4
#define _UA_FORCE_UNWIND        8
#define _UA_END_OF_STACK        16
#define _URC_NO_REASON  _URC_OK

extern "C" int printf(...);

extern "C" _Unwind_Reason_Code PERSONALITY_FUNCTION (int version,
                      _Unwind_Action actions,
                      _Unwind_Exception_Class exception_class,
                      struct _Unwind_Exception *ue_header,
                      struct _Unwind_Context *context)
{
	printf("ok\n");
	return _URC_OK;
}

extern "C" void _Unwind_SjLj_Register()
{
}

extern "C" void _Unwind_SjLj_Resume()
{
}

extern "C" void _Unwind_SjLj_Unregister()
{
}
