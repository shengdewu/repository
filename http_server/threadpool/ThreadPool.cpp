#include "ThreadPool.h"
#include "Thread.h"
#include "ScopedMutexLock.h"
#include <ctime>
#include <iostream>
#include "Logger.h"

//#define __CALLBACK;

class PooledThread : public Runable
{
public:
	
	//typedef void (*Task)(void *);
#ifdef __CALLBACK
	typedef std::tr1::function<void()> TargetPtr;
#else
	typedef Runable*				   TargetPtr;	
#endif

public:
	PooledThread();
	~PooledThread();

	void start();
	bool start(TargetPtr target);
	///
	void release();
	///线程空闲时调用
	void join();
	///等待run 执行完成,
	int  idleTime();
	bool idle();
	void activate();
	void run();
	
private:
	Condition	_targetReady;
	Condition   _started;
	Condition	_targetComplete;
	MutexLock	_mutex;
	TargetPtr	_pTarget;
	Thread		_thread;
	std::time_t _idleTime;
	bool		_idle;
};

PooledThread::PooledThread():
	_pTarget(nullptr),
	_idleTime(0),
	_idle(true)
{
	_idleTime = std::time(nullptr);
}

PooledThread::~PooledThread()
{

}

void PooledThread::start()
{
	_thread.start(*this);
	_started.wait();
}

bool PooledThread::start(TargetPtr pTarget)
{
	ScopedMutexLock lock(&_mutex);
	if(nullptr == pTarget)
	{
		return false;
	}

	_pTarget = pTarget;

	_targetReady.notify();

	return true;
}



void PooledThread::join()
{
	_mutex.lock();
	bool bjoin = false;
	TargetPtr pTarget = _pTarget;
	if(pTarget)
		bjoin = true;
	_mutex.unlock();
	if(bjoin)
		_targetComplete.wait();
}

void PooledThread::release()
{
	{
		ScopedMutexLock lock(&_mutex);
		_pTarget = nullptr;

		_targetReady.notify();
	}
	_thread.join();

	delete this;
}

int PooledThread::idleTime()
{
	ScopedMutexLock lock(&_mutex);
	return (std::time(nullptr) - _idleTime);
}

bool PooledThread::idle()
{
	return _idle;
}

void PooledThread::activate()
{
	ScopedMutexLock lock(&_mutex);
	_idle = false;
}

void PooledThread::run()
{
	_started.notify();
	while(true)
	{
		_targetReady.wait();
		_mutex.lock();
		if(_pTarget)
		{
			//_idle = false;
			///在这个地方置位，容易造成这个线程被其他对象抢占，
			///比如两个start 间隔小，因为_idle还未被复位。
			_mutex.unlock();

#ifdef __CALLBACK
			_pTarget();
#else
			_pTarget->run();
#endif
			ScopedMutexLock lock(&_mutex);
			_pTarget = nullptr;
			_idleTime = std::time(nullptr);
			_idle = true;
			_targetComplete.notify();
		}
		else
		{
			_mutex.unlock();
			break;
		}
	}
}

ThreadPool::ThreadPool(int minCapacity,
	int maxCapacity,
	int idleTime,
	int stackSize):
	_minCapacity(minCapacity), 
	_maxCapacity(maxCapacity), 
	_idleTime(idleTime),
	_serial(0),
	_stackSize(stackSize),
	_age(0)
{
	if(_maxCapacity < _minCapacity || _minCapacity < 1)
		throw Exception("ThreadPool", "thread capacity is small");

	for(int i=0; i < _minCapacity; i++)
	{
		PooledThread *pThread = new PooledThread();
		_thread.push_back(pThread);
		pThread->start();
	}

	logger_log(Logger::getLogger("Http.log"), "The thread pool is start up");
}

ThreadPool::~ThreadPool()
{

}

bool ThreadPool::start(Task task)
{
	PooledThread *pThread = getThread();
	if(nullptr == pThread)
	{
		return false;
	}
#ifdef __CALLBACK
	return pThread->start(task);
#else
	return false;
#endif

}

bool ThreadPool::start(Runable* pTarget)
{
	PooledThread *pThread = getThread();
	if(nullptr == pThread)
	{
		return false;
	}

#ifndef __CALLBACK
	return pThread->start(pTarget);
#else
	return false;
#endif
}

void ThreadPool::stopAll()
{
	ScopedMutexLock lock(&_mutex);
	std::vector<PooledThread*>::iterator it = _thread.begin();
	for(; it != _thread.end(); it ++)
	{
		(*it)->release();
	}

	_thread.clear();
}

void ThreadPool::joinAll()
{
	ScopedMutexLock lock(&_mutex);
	std::vector<PooledThread*>::iterator it = _thread.begin();
	for(; it != _thread.end(); it ++)
	{
		(*it)->join();
	}

	houseKeep();
}

PooledThread* ThreadPool::getThread()
{
	ScopedMutexLock lock(&_mutex);
	if(++_age == 32)
		houseKeep();

	PooledThread *pThread = nullptr;
	ThreadVecIter it = _thread.begin();
	for(; it != _thread.end(); it ++)
	{
		if((*it)->idle())
		{
			pThread = *it;
			break;
		}
	}

	if(!pThread)
	{
		if(_thread.size() < _maxCapacity)
		{
			pThread = new PooledThread();
			try
			{
				pThread->start();
				_thread.push_back(pThread);
			}
			catch(...)
			{
				delete pThread;
				pThread = nullptr;
			}
		}
	}
	if(pThread)
		pThread->activate();
	return pThread;
}

void ThreadPool::houseKeep()
{
	_age = 0;
	if(_thread.size() <= _minCapacity)
	{
		return;
	}

	ThreadVec activeThreads;
	ThreadVec idleThreads;
	ThreadVec expiredThreads;
	ThreadVecIter it = _thread.begin();
	for(; it != _thread.end(); it++)
	{
		if((*it)->idle())
		{
			if( _idleTime >= (*it)->idleTime())
				expiredThreads.push_back(*it);
			else
				idleThreads.push_back(*it);
		}
		else activeThreads.push_back(*it);
	}

	int limit = idleThreads.size() + activeThreads.size();

	limit = limit < _minCapacity ? _minCapacity : limit;

	idleThreads.insert(idleThreads.end(), expiredThreads.begin(), expiredThreads.end());
	_thread.clear();

	int n = 0;
	for(it = idleThreads.begin(); it != idleThreads.end(); it++)
	{
		if(n < limit)
		{
			_thread.push_back(*it);
			n++;
		}
		else (*it)->release();
	}

	_thread.insert(_thread.end(), activeThreads.begin(), activeThreads.end());
}


void ThreadPool::collect()
{
	ScopedMutexLock lock(&_mutex);
	houseKeep();
}

int ThreadPool::allocate()
{
	ScopedMutexLock lock(&_mutex);
	return _thread.size();
}
