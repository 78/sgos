
#include <api.h> //SGOS2 native API
#include <system.h>

namespace System{
	
	/* 静态函数 */
	//返回当前线程
	Thread Thread::ThisThread()
	{
		Thread thread;
		thread.handle = (uint)sys_thread_self();
		return thread;
	}
	
	//当前线程等待
	void Thread::Sleep(uint ms)
	{
		sys_thread_wait(ms);
	}
	
	//结束当前线程
	void Thread::Exit(uint code)
	{
		sys_thread_exit(code);
	}
	
	//查找一个线程
	Thread Thread::find( const char* name )
	{
		uint hdl;
		Thread thread;
		hdl = match_name( name );
		if( hdl == 0 ){
			//An exception should happen here...
		}
		thread.handle = hdl;
		return thread;
	}
	
	Thread::Thread()
	{
		this->procedure = NULL;
		this->exitCode = 0;
		this->handle = 0;
	}

	Thread::Thread(void* proc)
	{
		this->procedure = proc;
		this->exitCode = 0;
		this->handle = 0;
	}
	
	Thread::~Thread()
	{
		//detach the thread??
	}

	/* 非静态函数 */
	// 终止线程
	void Thread::abort()
	{
		sys_thread_terminate( this->handle, -1 );
	}
	
	// 开始线程
	void Thread::resume()
	{
		sys_thread_resume( this->handle );
	}
	
	void Thread::start()
	{
		int ret;
		ret = sys_thread_create( (uint)this->procedure, &this->handle );
		if( ret<0 )
			return;
		resume();
	}
	
	// 挂起线程
	void Thread::suspend()
	{
		sys_thread_suspend( this->handle );
	}
	
	// 等待线程结束
	void Thread::join()
	{
		//not implemented yet!
	}
	
	// 获取句柄
	uint Thread::getHandle()
	{
		return this->handle;
	}
	
	//命名系统
	int Thread::createName( const char* name )
	{
		return create_name( this->handle, name );
	}
	
	void Thread::deleteName( const char* name )
	{
		delete_name( this->handle, name );
	}
	
}
