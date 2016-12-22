#include <iostream>
#include "Thread.h"
#include "Condition.h"

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

int main(int argc, char *argv[])
{
	Thread thread;
	MyRunable	r;
	thread.start(r);
	while(!r.isRun());
	r.notify();
	thread.join();
	while(!thread.isRunning());
	std::cout << "MyRunable's cnt = " << r.getCnt() << std::endl;
	return 0;
}