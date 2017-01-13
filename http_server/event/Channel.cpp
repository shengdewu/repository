#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"

const int	Channel::KNONEVENT = 0;
const int	Channel::KREADEVENT = EPOLLIN | EPOLLPRI;
const int	Channel::KWRITEEVENT = EPOLLOUT;

Channel::Channel(int ifd, EventLoop *pLoop):
	_events(KNONEVENT),
	_fd(ifd),
	_pLoop(pLoop),
	_revents(KNONEVENT),
	_index(-1)
{
}


Channel::~Channel(void)
{
}

void Channel::handleEvent()
{

}

void Channel::remove()
{
	if(_pLoop)
	{
		_pLoop->removeChannel(this);
	}
}

void Channel::update()
{
	if(_pLoop)
	{
		_pLoop->updateChannel(this);
	}
}


