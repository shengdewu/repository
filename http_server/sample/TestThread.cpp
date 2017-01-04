#include <iostream>
#include "Thread.h"
#include "Condition.h"
#include "Runable.h"

class MyRunable : public Runable
{
public:
	MyRunable():_cnt(0),_run(false){}
	~MyRunable(){}

	void run()
	{
		for(int i=0; i< 20; i ++)
		{
			_cnt += i;
		}
		_run = true;
		_con.wait();
	}

	long getCnt()
	{
		return _cnt;
	}

	void notify()
	{
		_con.notify();
	}

	bool isRun()
	{
		return _run;
	}

private:
	Condition	_con;
	int			_cnt;
	bool		_run;
};

void Run(void *pThis)
{
	std::cout << "this is test cb" << std::endl;
}

int main(int argc, char *argv[])
{
	Thread thread;
	MyRunable	r;
	thread.start(r);
	while(!r.isRun());
	r.notify();
	thread.join();
	while(thread.isRunning());
	std::cout << "MyRunable's cnt = " << r.getCnt() << std::endl;

	//std::tr1::function<void (void*)>	func;
	//func = Run;
	thread.start(Run, nullptr);
	thread.join();
	while(thread.isRunning());
	
	return 0;
}
