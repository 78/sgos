#ifndef _APIDEF_H_
#define _APIDEF_H_

#include <types.h>

//system call for SGOS2
//调试
SYSCALL0( 0, uint, clock );
SYSCALL1( 1, int, dprint, const char*, buf );
//消息处理
SYSCALL2( 2, int, msg_send, const char*, buf, size_t, len );
SYSCALL3( 3, int, msg_recv, char*, buf, size_t, buf_siz, uint, flag );
//内存管理
SYSCALL1( 4, void*, virtual_alloc, size_t, siz );
SYSCALL1( 5, void, virtual_free, void*, p );
//线程管理
SYSCALL1( 6, void, thread_exit, int, code );	//退出当前线程
SYSCALL1( 7, int, thread_create, size_t, addr );	//创建线程，成功则返回线程id退出当前线程
SYSCALL0( 8, int, thread_self );	//返回当前线程ID
SYSCALL1( 9, int, thread_detach, int, id );	//脱离线程
SYSCALL2( 10, int, thread_join, int, id, int*, code );	//等待线程结束
SYSCALL1( 11, int, thread_wait, time_t, ms );	//线程睡眠一段时间
SYSCALL1( 12, int, thread_suspend, int, id );	//挂起线程
SYSCALL1( 13, int, thread_resume, int, id );	//启动线程
SYSCALL2( 14, int, thread_terminate, int, id, int, code );	//结束线程
SYSCALL2( 15, int, thread_set_priority, int, id, int, pri );	//设置线程优先级
SYSCALL2( 16, int, thread_get_priority, int, id, int*, pri );	//获取线程优先级
//进程管理
SYSCALL3( 20, int, process_create, char*, file, void*, environment, void*, create_info );
SYSCALL2( 21, int, process_terminate, int, id, int, code );	//结束进程
SYSCALL1( 22, int, process_suspend, int, id );	//挂起进程 
SYSCALL1( 23, int, process_resume, int, id );	//启动进程
SYSCALL0( 24, int, process_self );	//当前进程ID
//加载器
SYSCALL1( 25, int, loader_load, char*, file );	//返回库id
SYSCALL1( 26, int, loader_unload, int, id );	//卸载库
SYSCALL2( 27, size_t, loader_get_proc, int, id, char*, name );	//获得过程

//命名空间
SYSCALL2( 28, int, namespace_register, int, tid, char*, name );
SYSCALL2( 29, int, namespace_unregister, int, tid, char*, name );

//特权调用
SYSCALL2( 30, int, iomap_get, uchar*, buf, size_t, buf_size );
SYSCALL2( 31, int, iomap_set, uchar*, buf, size_t, buf_len  );
SYSCALL2( 32, int, irq_register, int, tid, int, irq );
SYSCALL2( 33, int, irq_unregister, int, tid, int, irq );
SYSCALL3( 34, int, virtual_map, size_t, vaddr, size_t, paddr, size_t, map_size );

#endif //_APIDEF_H_
