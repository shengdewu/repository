#include "Socketer.h"
#include "EventLoop.h"
#include "Channel.h"
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include "Logger.h"
#include "ThreadPool.h"

class TestEvent
{
public:
	TestEvent();
	~TestEvent();

	void test(int argc, char *argv[]);
	void stop();

private:
	void connect();
	void send();
	void read();
	void close();
	void error();
	void run();

	const char * const LOGER_NAME = "TestEvent.log";
	Channel		*_pChannel;
	EventLoop	*_pLoop;
	Socketer	_server;
	std::map<int, Channel*>	_listen;
	int			_fd;
	ThreadPool	_thread;
};

TestEvent::TestEvent():
	_pChannel(nullptr),
	_pLoop(nullptr)
{
}

TestEvent::~TestEvent()
{

}

void TestEvent::test(int argc, char *argv[])
{
	int port = atoi(argv[2]);
	char *ip = argv[1];

	int _fd = _server.listen(ip, port);
	if(-1 == _fd)
	{
		LOGGER_STREAM(Logger::getLogger(LOGER_NAME)) << "the server listen is failed!" << "\n";
	}

	std::function<void (void)> frun = std::bind(&TestEvent::run, this);
	_thread.start(frun);

}

void TestEvent::stop()
{
	_pLoop->stop();
	_thread.joinAll();
	_thread.stopAll();
	_pChannel->disableAlling();

	delete _pChannel;
	delete _pLoop;
	Logger::releseLogger(LOGER_NAME);
}

void TestEvent::connect()
{
	LOGGER_STREAM(Logger::getLogger(LOGER_NAME)) << "the server detect a new connect" << "\n";
	int fd = _server.accept();

	if(fd < 0 )
	{
		LOGGER_STREAM(Logger::getLogger(LOGER_NAME)) << "the server accept is failed!" << fd << "\n";	
	}
	else
	{
		LOGGER_STREAM(Logger::getLogger(LOGER_NAME))  << "the server accpet is success!" << fd << "\n";
	}
	Channel *pChannel = new Channel(fd, _pLoop);
	_listen.insert(std::pair<int, Channel*>(fd, pChannel));

	std::function<void (void)> fread = std::bind(&TestEvent::read, this);
	pChannel->setReadCallback(fread);
	std::function<void (void)> ferror = std::bind(&TestEvent::error, this);
	pChannel->setErrorCallback(ferror);
	std::function<void (void)> fcolse = std::bind(&TestEvent::close, this);
	pChannel->setCloseCallback(fcolse);
	pChannel->enableReading();
}

void TestEvent::send()
{

}

void TestEvent::read()
{

}

void TestEvent::close()
{

}

void TestEvent::error()
{

}

void TestEvent::run()
{
	_pLoop = new EventLoop();
	_pChannel = new Channel(_fd, _pLoop);
	
	std::function<void (void)> func = std::bind(&TestEvent::connect, this);
	_pChannel->setReadCallback(func);
	_pChannel->enableReading();
	_pLoop->loop();
}


int main(int argc, char *argv[])
{
	TestEvent test;
	test.test(argc, argv);

	char c = '\0';
	std::cin >> c;
	while(c != 'q')
	{
		std::cin >> c;
	}
	test.stop();

	return 0;
}
