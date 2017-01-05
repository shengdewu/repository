#pragma once

class SocketImp
{
public:
	SocketImp();
	~SocketImp();

	int listen(const char *ip, const int port, int backlog = 0);
	int accept(const int fd);
	int send(const int fd, const char *pbuff, const int len);
	int read(const int fd, char *pbuff, const int len);
	void close(const int fd);

private:
	int setOpt(const int fd);
	
private:
	const int   SOCKET_ERR;
	const int   MAX_BACKLOG;
private:
	int	  _fd;
};
