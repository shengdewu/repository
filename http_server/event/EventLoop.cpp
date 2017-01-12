#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"

EventLoop::EventLoop(void):
	_ePoll(new EPoller())
{
}


EventLoop::~EventLoop(void)
{

}

void EventLoop::updateChannel(Channel *pChannel)
{
	_ePoll->updateChannel(pChannel);
}
