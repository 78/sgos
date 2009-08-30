//semaphore

#include <sgos.h>
#include <arch.h>
#include <debug.h>
#include <string.h>
#include <thread.h>
#include <semaphore.h>
#include <mm.h>


//尝试P
int sema_trydown( sema_t *sem )
{	
	uint eflags;
	//we don't want to be interrupt
	local_irq_save( eflags );
	while( --sem->value ){
		//don't have it..
		local_irq_restore(eflags);
		sem->value ++;
		return 0;
	}
	//have it
	local_irq_restore(eflags);
	return 1;
}

//P
void sema_down( sema_t *sem )
{
	uint eflags;
	//we don't want to be interrupt
	local_irq_save( eflags );
	if( --sem->value ){
		//dont have it, then sleep
		//跟队末尾
		queue_push_front( &sem->wait_queue, current_thread() );
		thread_sleep();
		//now, we have it!!
	}
	local_irq_restore(eflags);
}

//初始化
void sema_init( sema_t *sem )
{
	sema_init_ex( sem, 1 );
}

//初始化
void sema_init_ex( sema_t *sem, int v )
{
	queue_create( &sem->wait_queue, 0, NULL, "sema_queue", 0 );
	sem->value = v;
}

//V
void sema_up( sema_t *sem )
{
	uint eflags;
	THREAD* thr = NULL;
	local_irq_save( eflags );
	thr = queue_pop_back( &sem->wait_queue );
	local_irq_restore( eflags );
	sem->value ++;
	if( thr ){
		thread_wakeup( thr );
	}
}

//释放一个sema
void sema_destroy( sema_t *sem )
{
	THREAD* thr;
	uint eflags;
	local_irq_save( eflags );
	while( (thr=queue_pop_back(&sem->wait_queue)) ){
		thread_wakeup( thr );
	}
	sem->value = 0;
	local_irq_restore( eflags );
}

//删除一个链表中的项
void sema_remove_thread( sema_t* sem, THREAD* thr )
{
	uint eflags;
	qnode_t *nod;
	local_irq_save(eflags);
	thr = queue_quick_search( &sem->wait_queue, thr, &nod );
	if( thr )
		queue_remove( &sem->wait_queue, nod );
	else
		PERROR("##thread not found.");
	local_irq_restore(eflags);
}
