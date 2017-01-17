#pragma once
#include <string>

class MsgText
{
public:
	MsgText(void);
	~MsgText(void);
	void setFile(const std::string &file, const int line);
	std::string format();
	std::string format(const std::string &context);

private:
	std::string  _file;
	int			 _line;
	long long	  _time;
};

