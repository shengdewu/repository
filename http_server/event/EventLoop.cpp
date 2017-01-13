#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"
#include <sys/eventfd.h>
#include "Logger.h"
#include "Socketer.h"

EventLoop::EventLoop(void):
	_ePoll(new EPoller()),
	_wakeupFd(createEventfd()),
	_wakeupChannel(new Channel(_wakeupFd, this)),
	_threadID(Thread::currentThreadID())

{
	_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
}


EventLoop::~EventLoop(void)
{
	Logger::releseLogger(LOGGER_NAME);
}

void EventLoop::loop()
{
	if(!isInLoopThread())
	{
		return;
	}

	_loop = true;
	while(_loop)
	{
		_activeChannel.clear();
		int nret = _ePoll->poll(KEPOLL_TIMES, _activeChannel);
		
		ChannelListIter iter = _activeChannel.begin();
		for(; iter != _activeChannel.end(); iter ++)
		{
			(*iter)->handleEvent();
		}

		doPendingFunctor();
	}
}

void EventLoop::stop()
{
	_loop = false;
}

void EventLoop::updateChannel(Channel *pChannel)
{
	_ePoll->updateChannel(pChannel);
}

void EventLoop::removeChannel(Channel *pChannel)
{
	_ePoll->removeChannel(pChannel);
}

void EventLoop::runInLoop(const Functor &cb)
{
	if(isInLoopThread())
	{
		cb();
	}
	else
	{
		queueFunctor(std::move(cb));
	}
}

int EventLoop::createEventfd()
{
	int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if(fd < 0)
	{
		logger_log(Logger::getLogger(LOGGER_NAME), "filed in eventfd");
		abort();
	}
	return fd;
}

void EventLoop::wakeup()
{
	const char one = 'w';
	Socketer wake;
	wake.send(_wakeupFd, &one, sizeof(one));
	logger_log(Logger::getLogger(LOGGER_NAME), "wakeup");
}

void EventLoop::handleRead()
{
	char one;
	Socketer wake;
	wake.read(_wakeupFd, &one, sizeof(one));
	logger_log(Logger::getLogger(LOGGER_NAME), "handleRead");
}

void EventLoop::doPendingFunctor()
{
	Functors functors;
	{
		ScopedMutexLock lock(&_mutex);
		functors.swap(_pendingFunctors);
	}

	for(int i=0; i<functors.size(); i++)
	{
		functors[i]();
	}
}

void EventLoop::queueFunctor(const Functor &cb)
{
	ScopedMutexLock lock(&_mutex);
	_pendingFunctors.push_back(std::move(cb));

	if(!isInLoopThread())
	{
		wakeup();
	}
}