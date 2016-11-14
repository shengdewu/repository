#pragma once

namespace EventConst
{
	const int EVENT_ADD = 0;
	const int EVENT_DEL = 1;
	const int EVENT_MODIFY = 2;

	const int EVENT_READ = 0;
	const int EVENT_WRITE = 1;
}

class Event
{
public:
	Event(int fd = 0, int opt = 0, int event = 0);
	~Event();
	Event & operator = (const Event &ev);
	int Fill(int fd, int opt, int event);
	int Setfd(int fd);
	int Getfd();
	int SetOpt(int opt);
	int GetOpt();
	int SetEvent(int ev);
	int GetEvent();

private:
	int		m_fd;
	int		m_opt;
	int     m_event;
};
