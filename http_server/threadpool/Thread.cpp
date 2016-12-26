#include "Thread.h"
#include <cstring>
#include <iostream>

Thread::ThreadData::ThreadData():
	pRunableTarget(nullptr),
	pCallbackTarget(nullptr),
	pAgrs(nullptr)
{
	std::memset(&thread, 0, sizeof(pthread_t));
}

Thread::ThreadData::~ThreadData()
{
	pRunableTarget = nullptr;
	if(pCallbackTarget)
	{
		delete pCallbackTarget;
		pCallbackTarget = nullptr;
	}
	pAgrs = nullptr;
}

Thread::Thread():
	_pData(new ThreadData)
{
}

Thread::~Thread()
{
	if(isRunning())
		pthread_detach(_pData->thread);
}

void Thread::start(Runable &target)
{
	if(_pData->pRunableTarget)
		return;

	_pData->pRunableTarget = &target;

	if(pthread_create(&(_pData->thread), nullptr, runableEntry, this))
	{
		_pData->pRunableTarget = nullptr;
		std::cout << "Failed int pthread_create" << std::endl;
	}
}

void Thread::start(Callable target, void *pData)
{
	if(_pData->pCallbackTarget && _pData->pCallbackTarget->callback)
		return;

	_pData->pCallbackTarget = new struct CallbackData();
	_pData->pCallbackTarget->callback = target;
	_pData->pCallbackTarget->pData = pData;

	if(pthread_create(&(_pData->thread), nullptr, callableEntry, this))
	{
		_pData->pRunableTarget = nullptr;
		std::cout << "Failed int pthread_create" << std::endl;
	}
}

void Thread::join()
{
	_cond.wait();
	void *result;
	if(pthread_join(_pData->thread, &result))
		throw Exception("cannot join thread");
}


bool Thread::isRunning()
{
	if((_pData->pCallbackTarget && _pData->pCallbackTarget->callback)
		|| _pData->pRunableTarget)
		return true;

	return false;
}

void *Thread::runableEntry(void *pT)
{
	Thread *pThread = reinterpret_cast<Thread*>(pT);
	std::tr1::shared_ptr<ThreadData> pData =  pThread->_pData;

	pData->pRunableTarget->run();

	pData->pRunableTarget = nullptr;

	pThread->_cond.notifyAll();
	return nullptr;
}

void *Thread::callableEntry(void *pT)
{
	Thread *pThread = reinterpret_cast<Thread*>(pT);
	std::tr1::shared_ptr<ThreadData> pData = pThread->_pData;

	pData->pCallbackTarget->callback(pData->pCallbackTarget->pData);

	pData->pCallbackTarget = nullptr;

	return nullptr;
}