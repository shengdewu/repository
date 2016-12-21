#pragma once

class SocketImp
{
public:
	SocketImp();
	~SocketImp();

	int Listen(const char *ip, const int port, int backlog = 0);
	int Accept(const int fd);
	int Send(const int fd, const char *pbuff, const int len);
	int Read(const int fd, char *pbuff, const int len);
	void Close(const int fd);

private:
	int SetOpt(const int fd);
	
private:
	const int   SOCKET_ERR;
	const int   MAX_BACKLOG;
private:
	int	  m_fd;
};
