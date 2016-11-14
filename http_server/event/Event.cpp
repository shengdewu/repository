#include <sys/epoll.h>
#include "Event.h"

Event::Event(int fd, int opt, int event):
	m_fd(fd),
	m_opt(opt),
	m_event(event)
{
}

Event::~Event()
{

}

int Event::Setfd(int fd)
{
	m_fd = fd;
	return m_fd;
}

int Event::Getfd()
{
	return m_fd;
}

int Event::SetOpt(int opt)
{
	m_opt = opt;
	return m_opt;
}

int Event::GetOpt()
{
	return m_opt;
}

int Event::SetEvent(int event)
{
	m_event = event;
	return event;
}

int Event::GetEvent()
{
	return m_event;
}


Event & Event::operator=(const Event &ev)
{
	this->m_fd = ev.m_fd;
	this->m_opt = ev.m_opt;
	this->m_event = ev.m_event;

	return (*this);
}

int Event::Fill(int fd, int opt, int event)
{
	m_fd = fd;
	m_opt = opt;
	m_event = event;

	return fd;
}

