typedef struct FUNC_LIST{
	struct FUNC_LIST*	next;
	void*			func;
}FUNC_LIST;

static FUNC_LIST* first;

extern void* malloc(unsigned int );
extern void free(void*p);
//Not support multithreading yet.
extern "C" void atexit(void* p)
{
	FUNC_LIST* f = (FUNC_LIST*)malloc( sizeof(FUNC_LIST) );
	if( f==(void*)0 ) return;
	f->func = p;
	//Note: Multithreading is not supported here!!
	f->next = first;
	first = f;
}

extern "C" void __run_exit_func()
{
	FUNC_LIST* f;
	for( f=first; f; f=f->next )
		if(f->func)
			((void(*)())f->func)();
}
