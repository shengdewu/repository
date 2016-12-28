#include "ThreadPool.h"
#include "Thread.h"

ThreadPool::ThreadPool(int minCapacity,
	int maxCapacity,
	int idleTime,
	int stackSize):
	_minCapacity(minCapacity), 
	_maxCapacity(maxCapacity), 
	_idleTime(idleTime),
	_serial(0),
	_stackSize(stackSize)
{
	_thread.clear();
}

ThreadPool::~ThreadPool()
{

}

void ThreadPool::start(int numThread)
{
	numThread  = numThread > _maxCapacity ? _maxCapacity : numThread;
	numThread  = numThread < _minCapacity ? _minCapacity : numThread;

	for(int i=0; i< numThread; i++)
	{
		Thread *pThread = new Thread();
	}
}

void ThreadPool::runThred(void *pThis)
{

}