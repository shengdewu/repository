#include <iostream>
#include "ThreadPool.h"
#include "Condition.h"

void run(int n)
{
	std::cout << "thread num = " << n << std::endl;
}
class TestThreadPool
{
public:
	TestThreadPool():_cnt(0)
	{}
public:
	void run(int number)
	{
		_cond.wait();
		for(int i =0; i < 1000; i ++)
		{
			_mutex.lock();
			std::cout << "thread num = " << number << std::endl;
			_cnt ++;
			_mutex.unlock();
		}
	}

	void testPool()
	{
		ThreadPool tpool;
		int argc = 2;
		std::cout << "==========test fucn===num = "<< argc << std::endl;

		for(int i = 0; i < argc; i ++)
		{
			std::function<void()> trun;
			trun = std::bind(run, i, std::placeholders::_1);
			tpool.start(trun);
		}

		tpool.collect();
		std::cout << "thread pool size = " << tpool.allocate() << std::endl;
		tpool.joinAll();
		tpool.stopAll();

		argc = 10;
		std::cout << "==========test member fucn===num = "<< argc << std::endl;
		for(int i = 0; i < argc; i ++)
		{
			std::function<void()> trun;
			trun = std::bind(&TestThreadPool::run, *this, i, std::placeholders::_1, std::placeholders::_2);
	
			tpool.start(trun);
			_cond.notify();
		}

		argc = 20;
		std::cout << "==========test member fucn===num = "<< argc << std::endl;
		for(int i = 0; i < argc; i ++)
		{
			std::function<void()> trun;
			trun = std::bind(&TestThreadPool::run, *this, i, std::placeholders::_1, std::placeholders::_2);
			tpool.start(trun);
			_cond.notify();
		}
	}
private:
	MutexLock	_mutex;
	Condition   _cond;
	long		_cnt;
};

int main(int argc, char *argv[])
{
	
	TestThreadPool tpool;
	tpool.testPool();
	return 0;
}