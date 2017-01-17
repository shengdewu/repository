#include <iostream>
#include "ThreadPool.h"
#include "Condition.h"
#include <ctime>
#include "Logger.h"

void srun(int n)
{
	std::cout << "thread num = " << n << std::endl;
}
class TestThreadPool : public Runable
{
public:
	TestThreadPool():_cnt(0)
	{}
public:
	void run(int number)
	{
		//_cond.wait();
		std::cout << "thread num = " << number << std::endl;
		for(int i =0; i < 1000; i ++)
		{
			_mutex.lock();
			_cnt ++;
			_mutex.unlock();
		}
	}

	void run()
	{
		//_cond.wait();
		std::cout << "thread num = " << _number<< "current time :"<< std::time(nullptr) << std::endl;
		std::time_t ts = std::time(nullptr);
		for(int i =0; i < 10; i ++)
		{
			_mutex.lock();
			_cnt ++;
			std::time_t cr = std::time(nullptr);
			while((cr - ts) < 2)
			{
				cr = std::time(nullptr);
			}
			ts = std::time(nullptr);
			_mutex.unlock();
		}
	}
	void setNumber(int number)
	{
		_number = number;
	}

	void testPool()
	{
		ThreadPool tpool(3, 20);
		int argc = 2;
		std::cout << "==========test fucn===num = "<< argc << std::endl;

		time_t ts = std::time(nullptr);
		for(int i = 0; i < argc; i ++)
		{
			std::tr1::function<void()> trun;
			trun = std::tr1::bind(srun, i);
			tpool.start(trun);
			/*
			std::time_t cr = std::time(nullptr);
			while((cr - ts) < 2)
			{
				cr = std::time(nullptr);
			}
			ts = std::time(nullptr);
			*/
		}

		tpool.collect();
		std::cout << "thread pool size = " << tpool.allocate() << std::endl;
		tpool.joinAll();
		tpool.stopAll();

		argc = 30;
		std::cout << "==========test member fucn===num = "<< argc << std::endl;
		for(int i = 0; i < argc; i ++)
		{
			//std::tr1::function<void()> trun;
			//trun = std::tr1::bind(&TestThreadPool::run, this, i);

			setNumber(i);

			bool success = false;
			unsigned long cnt = 0;
			while(!success && cnt < 3)
			{
				success = tpool.start(this);
				ts = std::time(nullptr);
				if(success)
				{
					std::cout <<"===new add thread id = "<< i <<",current thread size = "<< tpool.allocate() << std::endl;
				}
				else
				{
					std::cout <<"===add thread failed id = "<< i <<",current thread size = "<< tpool.allocate() << std::endl;
					std::time_t cr = std::time(nullptr);
					while((cr - ts) < 10)
					{
						cr = std::time(nullptr);
					}
					cnt ++;
					tpool.collect();
					std::cout << "thread pool size = "<< tpool.allocate() <<",the cnt = "<< _cnt << std::endl;

				}
			}

			/*
			std::time_t cr = std::time(nullptr);
			while((cr - ts) < 2)
			{
				cr = std::time(nullptr);
			}
			ts = std::time(nullptr);
			*/
		}

		ts = std::time(nullptr);
		std::time_t cr = std::time(nullptr);
		while((cr - ts) < 100)
		{
			cr = std::time(nullptr);
		}

		tpool.collect();
		std::cout << "thread pool size = "<< tpool.allocate() <<",the cnt = "<< _cnt << std::endl;

		tpool.joinAll();
		tpool.stopAll();

		LOGGER(Logger::getLogger("http.log")) << "the pool stop,thread pool size =" << "the cnt = " << _cnt << "\n";
		Logger::releseLogger("http.log");

	}
private:
	MutexLock	_mutex;
	Condition   _cond;
	long		_cnt;
	long		_number;
};

int main(int argc, char *argv[])
{
	
	TestThreadPool tpool;
	tpool.testPool();
	return 0;
}