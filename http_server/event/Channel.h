#pragma once
#include <functional>

class EventLoop;
class Channel
{
public:
	typedef std::function<void (void)>  EventCallback;

public:
	Channel(int ifd, EventLoop *pLoop = nullptr);
	~Channel(void);

	void enableReading();
	void enableWriteing();
	void disableReading();
	void disableWriteing();
	int	 fd();
	void setRevent(int re);
	void setIndex(int index);
	int  index();
	void setReadCallback(const EventCallback &readCallback);
	void setWriteCallback(const EventCallback &writeCallback);
	void setCloseCallback(const EventCallback &closereadCallback);
	void setErrorCallback(const EventCallback &errorCallback);
	void handleEvent();
	int events();
	void remove();
	bool isNonevent();

private:
	void update();

	static const int	KNONEVENT;
	static const int	KREADEVENT;
	static const int	KWRITEEVENT;

	int			_events;
	int			_fd;
	EventLoop*	_pLoop;
	int			_revents;
	int			_index; 
	///判断事件模式，new add delete

	EventCallback	_readCallback;
	EventCallback	_writeCallback;
	EventCallback	_closeCallback;
	EventCallback	_errorCallback;
};

//
//inline
//
inline void Channel::enableReading()
{
	_events |= KREADEVENT;
	update();
}

inline void Channel::enableWriteing()
{
	_events |= KWRITEEVENT;
	update();
}

inline void Channel::disableReading()
{
	_events &= ~KREADEVENT;
	update();
}

inline void Channel::disableWriteing()
{
	_events &= ~KWRITEEVENT;
	update();
}


inline int Channel::fd()
{
	return _fd;
}

inline void Channel::setRevent(int re)
{
	_revents = re;
}

inline void Channel::setIndex(int index)
{
	_index = index;
}

inline int Channel::index()
{
	return _index;
}

inline	void Channel::setReadCallback(const EventCallback &readCallback)
{
	_readCallback = readCallback;
}

inline	void Channel::setWriteCallback(const EventCallback &writeCallback)
{
	_writeCallback = writeCallback;

}

inline	void Channel::setCloseCallback(const EventCallback &closereadCallback)
{
	_closeCallback = closereadCallback;
}

inline	void Channel::setErrorCallback(const EventCallback &errorCallback)
{
	_errorCallback = errorCallback;
}

inline int Channel::events()
{
	return _events;
}


inline bool Channel::isNonevent()
{
	return (_events == KNONEVENT);
}