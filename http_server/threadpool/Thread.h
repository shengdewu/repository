#pragma once 
#include <pthread.h>
#include "AutoPtr.h"
#include "Runable.h"
#include "Condition.h"
#include <tr1/memory>

class Thread
{
public:
	typedef void(*Callable)(void *);
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
