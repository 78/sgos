// Name space for naming threads

#include <sgos.h>
#include <arch.h>
#include <message.h>
#include <namespace.h>
#include <queue.h>
#include <debug.h>
#include <mm.h>
#include <string.h>

#define MAX_NAME_IN_QUEUE	1024

//队列
queue_t	nq;
//用queue很不合适，可以做一个算法，把
//频繁访问的元素可以移动到前面来。

static void name_delete(const void* p)
{
	kfree((void*)p);
}

static int name_search(const void* p, const void* q )
{
	if( strncmp( ((name_t*)p)->name, q, NAME_LEN )== 0 )
		return 1;
	return 0;
}

void name_init()
{
	queue_create( &nq, MAX_NAME_IN_QUEUE, name_delete, "name_queue", 1 );
}

void name_destroy()
{
	queue_cleanup( &nq );
}

int name_insert( THREAD* thr, const char* name )
{
	name_t *n;
	if( strlen(name) > NAME_LEN )
		return -ERR_WRONGARG;
	n = kmalloc( sizeof(name_t) );
	if( n== NULL )
		return -ERR_NOMEM;
	strncpy( n->name, name, NAME_LEN );
	n->thread = thr;
	if( queue_push_front( &nq, n ) < 0 ){
		kfree( n );
		return -ERR_NOMEM;
	}
	return 0;
}

int name_remove( THREAD* thr, const char* name )
{
	name_t* n;
	qnode_t* nod;
	n = queue_search( &nq, (char*)name, name_search, &nod );
	if( !n )
		return -ERR_NONE;
	if( n->thread == thr )
		queue_remove( &nq, nod );
	name_delete(n);
	return 0;
}

void* name_match( const char* name )
{
	name_t* n;
	qnode_t* nod;
	n = queue_search( &nq, (char*)name, name_search, &nod );
	if( !n )
		return NULL;
	return n->thread;
}


