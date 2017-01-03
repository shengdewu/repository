#include "ThreadPool.h"
#include "Thread.h"
#include "ScopedMutexLock.h"
#include <ctime>

class PooledThread : public Runable
{
public:
	//typedef void (*Task)(void *);
	typedef std::function<void()> Task;
public:
	PooledThread();
	~PooledThread();

	void start();
	bool start(Task pTask);
	///
	void release();
	///线程空闲时调用
	void join();
	///等待run 执行完成,
	int  idleTime();
	bool idle();
	
	void run();
	
private:
	Condition	_targetReady;
	Condition   _started;
	Condition	_targetComplete;
	MutexLock	_mutex;
	Task		_pTask;
	Thread		_thread;
	std::time_t _idleTime;
	bool		_idle;
};

PooledThread::PooledThread():
	_pTask(nullptr),
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

bool PooledThread::start(Task pTask)
{
	ScopedMutexLock lock(&_mutex);
	if(nullptr == pTask)
	{
		return true;
	}

	_pTask = pTask;

	_targetReady.notify();
}

void PooledThread::join()
{
	_mutex.lock();
	Task task = _pTask;
	_mutex.unlock();
	if(task)
		_targetComplete.wait();
}

void PooledThread::release()
{
	{
		ScopedMutexLock lock(&_mutex);

		_pTask = nullptr;

		_targetReady.notify();
	}
	_thread.join();

	delete this;
}

int PooledThread::idleTime()
{
	return (std::time(nullptr) - _idleTime);
}

bool PooledThread::idle()
{
	return _idle;
}

void PooledThread::run()
{
	_started.notify();
	while(true)
	{
		_targetReady.wait();
		_mutex.lock();
		if(_pTask)
		{
			_idle = false;
			_mutex.unlock();
			_pTask();

			ScopedMutexLock lock(&_mutex);
			_pTask = nullptr;
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

	for(int i=0; i <= _minCapacity; i++)
	{
		PooledThread *pThread = new PooledThread();
		_thread.push_back(pThread);
		pThread->start();
	}
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

	pThread->start(task);
	return true;
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
	if(++_age == 32)
		houseKeep();

	ScopedMutexLock lock(&_mutex);
	PooledThread *pThread = nullptr;
	ThreadVecIter it = _thread.begin();
	for(; it != _thread.end(); it ++)
	{
		if((*it)->idle())
		{
			pThread = *it;
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
	for(; it != _thread.end();)
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