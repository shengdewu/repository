#include "EPoller.h"
#include "Channel.h"
#include <sys/epoll.h>
#include "Logger.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

namespace 
{
	const int KNEW = -1;
	const int KADD = 1;
	const int KDELETE = 2;
}

EPoller::EPoller(void):
	_epfd(::epoll_create1(EPOLL_CLOEXEC)),
	_events(KEVENTS_INITS)
{
}


EPoller::~EPoller(void)
{
	Logger::releseLogger(LOGGER_NAME);
}

int	 EPoller::poll(int timeout, ChannelList& activeChannel)
{
	activeChannel.clear();
	_events.clear();
	int numEvents = ::epoll_wait(_epfd, &(*_events.begin()), _events.size(), timeout);
	if(numEvents < 0)
	{
		LOGGER(Logger::getLogger(LOGGER_NAME)) << "errno = " << errno << strerror(errno) << "\n";
		return numEvents;
	}

	if(numEvents > 0)
	{
		fillChannelEvents(numEvents, activeChannel);
		if(numEvents >= _events.size())
		{
			_events.resize(numEvents << 1);
		}
	}

	return numEvents;
}

void EPoller::updateChannel(Channel *pChannel)
{
	if(nullptr == pChannel)
	{
		return;
	}

	int index = pChannel->index();
	if(KNEW == index || KDELETE == index)//channel 是新的，或者事件已被清除了的
	{
		int fd = pChannel->fd();
		ChannelMapIter iter = _channelMap.find(fd);
		if(KNEW == index)
		{
			if(iter == _channelMap.end())
			{
				_channelMap[fd] = pChannel;
			}			
		}
		else//kdelete
		{
			//此情况下，_channelMap应该有这个 channel，只是事件被卸载了
			assert(iter == _channelMap.end());
		}

		pChannel->setIndex(KADD);
		update(EPOLL_CTL_ADD, pChannel);
	}
	else //kADD
	{
		int fd = pChannel->fd();
		ChannelMapIter iter = _channelMap.find(fd);

		assert(iter != _channelMap.end());
		
		if(pChannel->isNonevent())
		{
			pChannel->setIndex(KDELETE);
			update(EPOLL_CTL_DEL, pChannel);
		}
		else
		{
			update(EPOLL_CTL_MOD, pChannel);
		}
	}
}

void EPoller::removeChannel(Channel *pChannel)
{
	if(nullptr == pChannel)
	{
		return;
	}
	int index = pChannel->index();
	int fd = pChannel->fd();
	ChannelMapIter iter = _channelMap.find(fd);
	assert(iter != _channelMap.end());
	_channelMap.erase(iter);
	assert(index == KADD || index == KDELETE);
	if(KADD == index && !pChannel->isNonevent())
	{
		update(EPOLL_CTL_DEL, pChannel);
	}
	pChannel->setIndex(KNEW);
}

void EPoller::update(int opt, Channel *pChannel)
{
	struct epoll_event ev;
	bzero(&ev, sizeof(struct epoll_event));
	ev.events = pChannel->events();
	ev.data.ptr = pChannel;
	//ev.events |= EPOLLET;
	int fd = pChannel->fd();
	if(::epoll_ctl(_epfd, opt, fd, &ev) < 0)
	{
		std::string strlog = createlogger(strlog, pChannel);
		LOGGER(Logger::getLogger(LOGGER_NAME)) << "epoll update is failed:"
				<< "opt = " << eventToString(opt) 
				<< ","<<strlog << "\n";
		
	}
}

void EPoller::fillChannelEvents(int num, ChannelList& activeChannel)
{
	if(num > _events.size())
	{
		return;
	}

	LOGGER(Logger::getLogger(LOGGER_NAME)) << "fillChannelEvents:" << "nums = " << num << "\n";

	for(int i=0; i<num; i++)
	{
		Channel* pChannel = static_cast<Channel*>(_events[i].data.ptr);
		pChannel->setRevent(_events[i].events);
		activeChannel.push_back(pChannel);
	}
}


std::string EPoller::createlogger(std::string strContext, Channel *pChannel)
{
	std::string strlog = strContext;
	std::string strtmp;
	std::stringstream s;
	s << pChannel->index();
	s >> strtmp;
	strlog.append("index = ").append(strtmp);
	strtmp.clear();
	s << pChannel->fd();
	s >> strtmp;
	strlog.append(",fd = ").append(strtmp);

	return strlog;
}

std::string EPoller::eventToString(int opt)
{
	switch(opt)
	{
		case EPOLL_CTL_ADD:
		  return "ADD";
		case EPOLL_CTL_DEL:
		  return "DEL";
		case EPOLL_CTL_MOD:
		  return "MOD";
		default:
		  return "Unknown Operation";
	}
}

