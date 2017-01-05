#pragma once
#include <string>

class MsgText
{
public:
	MsgText(void);
	~MsgText(void);
	void setFile(const std::string &file, const int line, const std::string &context);
	std::string format();

private:
	std::string  _file;
	int			 _line;
	std::string   _context;
	long long	  _time;
};

