//thread schedule

#include <sgos.h>
#include <arch.h>
#include <thread.h>
#include <process.h>
#include <debug.h>

THREAD_BOX tbox = {NULL, };
static const unsigned ms = 1000/RTC_FREQUENCY;

void sched_init()
{
	memset( &tbox, 0, sizeof(THREAD_BOX) );
//	tbox.running = current_proc()->thread;
}

THREAD* current_thread()
{
	return tbox.running;
}

THREAD** get_thread_link( enum THREAD_STATE st )
{
	switch( st ){
	case TS_INIT:
		break;
	case TS_DEAD:
		return &tbox.dead;
	case TS_RUNNING:
		return &tbox.running;
	case TS_READY:
		return &tbox.ready;
	case TS_SLEEP:
		return &tbox.sleep;
	case TS_PAUSED:
		return &tbox.paused;
	case TS_WAIT:
		return &tbox.wait;
	default:
		PERROR("##Warning: unknown thread state: 0x%X", st );
	}
	return NULL;
}

void dump_link()
{
	uint eflags;
	int i;
	local_irq_save(eflags);
	THREAD* thr;
	thr=current_proc()->thread;
	for( i=0; thr; thr=thr->next, i++ )
		;
	kprintf("(%d) ready list: ", i);
	for( i=0, thr=tbox.ready; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->id );
	kprintf("(%d)\n", i);
	kprintf("sleep list: ");
	for( i=0, thr=tbox.sleep; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->id );
	kprintf("(%d)\n", i);
	kprintf("wait list: ");
	for( i=0, thr=tbox.wait; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->id );
	kprintf("(%d)\n", i);
	kprintf("paused list: ");
	for( i=0, thr=tbox.paused; thr; i++, thr=thr->sched_info.next )	
		kprintf("[%d]", thr->id );
	kprintf("(%d)\n", i);
	local_irq_restore(eflags);
}

void sched_set_state( THREAD* thr, enum THREAD_STATE st )
{
	uint flags;
	THREAD** link;
	SCHEDULE_INFO *info;
	if( thr->state == st )
		return;
	local_irq_save( flags );
	info = &thr->sched_info;
	link = get_thread_link( thr->state );
	//remove from link
	if(link && *link){
		if(info->next)
			info->next->sched_info.pre = info->pre;
		if(info->pre){
			info->pre->sched_info.next = info->next;
		}else{
			*link = info->next;
		}
	} 
	info->pre = NULL;
	//put to link
	link = get_thread_link( st );
	if(link){
		//先来先占
		if( *link ){
			THREAD* t2;
			t2 = *link;
			while( t2->sched_info.next )
				t2 = t2->sched_info.next;
			t2->sched_info.next = thr;
			info->pre = t2;
		}else{
			*link = thr;
		}
		info->next = NULL;
/*
//		抢占式。。。。
		info->next = *link;
		if( *link )
			(*link)->sched_info.pre = thr;
		*link = thr;*/
	}
	thr->state = st;
//	kprintf("set state %d to %d\n", (*link)->id, st );
	//ok
	local_irq_restore( flags );
}


void sched_clock()
{
	THREAD* thr;
	SCHEDULE_INFO* info;
	
	for( thr=tbox.wait; thr; thr=thr->sched_info.next ){
		thr->sched_info.clock -= ms;
		if( thr->sched_info.clock<=0 ){
			sched_set_state( thr, TS_READY );
		}
	}
	local_irq_enable();
	thr = current_thread();
	info = &thr->sched_info;
	--info->clock;
	if( info->clock <= 0 ){	//need scheduling
		schedule();
	}
	local_irq_disable();
}

void schedule()
{
	THREAD* thr, *cur;
	SCHEDULE_INFO* info;
	uint flags;
	cur = current_thread();
	local_irq_save(flags);
	if( !cur ){
		cur = tbox.running = tbox.ready;
	}
	//next one
	thr = cur->sched_info.next;
	if(!thr || thr->state!=TS_READY)	//one round finished...
		thr = tbox.ready;
	if( thr ){
		if( thr != cur ){
			tbox.running = thr;
			thr->sched_info.clock = 1;
			switch_to( cur, thr );
		}
	}
	//back to me
	local_irq_restore(flags);
}


