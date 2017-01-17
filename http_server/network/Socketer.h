#pragma once
class Socketer
{
public:
	Socketer(void);
	~Socketer(void);

	int listen(const char *ip, const int port, int backlog = 0);
	int accept();
	int send(const int fd, const char *pbuff, const int len);
	int read(const int fd, char *pbuff, const int len);
	void close(const int fd);
	int	 fd();

private:
	int setOpt(const int fd);
	
private:
	const char * const LOGGER_NAME = "socket.log";

	const int   SOCKET_ERR;
	const int   MAX_BACKLOG;
private:
	int	  _fd;
};

//
//inline
//
inline int Socketer::fd()
{
	return _fd;
}
