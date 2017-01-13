#pragma once
#include "Thread.h"
#include <vector>
#include <functional>
#include "ScopedMutexLock.h"

class EPoller;
class Channel;
class EventLoop
{
public:
	typedef std::vector<Channel*>  ChannelList;
	typedef std::vector<Channel*>::iterator ChannelListIter;
	typedef std::function<void(void)>	Functor;
	typedef std::vector<Functor>		Functors;

public:
	EventLoop(void);
	~EventLoop(void);

	void loop();
	void stop();
	void updateChannel(Channel *pChannel);
	void removeChannel(Channel *pChannel);
	void runInLoop(const Functor &cb);

private:
	int	createEventfd();
	void wakeup();
	void handleRead();
	void doPendingFunctor();
	bool isInLoopThread() const;
	void queueFunctor(const Functor &cb);

	static const int  KEPOLL_TIMES = 10000;
	const char * const LOGGER_NAME = "EventLoop.log";

	EPoller			*_ePoll;
	ChannelList		_activeChannel;
	bool			_loop;
	int				_wakeupFd;
	Channel			*_wakeupChannel;
	unsigned long	_threadID;
	Functors		_pendingFunctors;
	MutexLock		_mutex;
};

//
//inline
//
inline bool EventLoop::isInLoopThread() const
{
	return (_threadID == Thread::currentThreadID());
}
