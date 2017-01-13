#pragma once 
#include <pthread.h>
#include "AutoPtr.h"
#include "Runable.h"
#include "Condition.h"
#include <functional>
#ifdef _WINDOWS
#include <memory>
#else
#include <tr1/memory>
#endif



class Thread
{
public:
	//typedef std::tr1::function<void (void *)>  Callable;
	typedef void (*Callable)(void *);

	struct CallbackData
	{
		CallbackData():callback(nullptr), pData(nullptr)
		{
		}
		Callable	callback;
		void        *pData;
	};
public:
	Thread();
	~Thread();

	void start(Runable &target);
	void start(Callable target, void *pData);
	void join();
	bool isRunning();
	static pthread_t currentThreadID();

private:
	static void* runableEntry(void *pT);
	static void* callableEntry(void *pT);

	struct ThreadData
	{
		ThreadData();
		~ThreadData();
		Runable			*pRunableTarget;
		CallbackData	*pCallbackTarget;
		pthread_t	thread;
		void		*pAgrs;
	};

	std::tr1::shared_ptr<ThreadData>	_pData;
	Condition			_cond;
};
