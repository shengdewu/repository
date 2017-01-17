#include "MsgText.h"
#include <ctime>
#include <sstream>

MsgText::MsgText(void):
	_line(0),
	_time(0)
{
	_time = time(nullptr);
}


MsgText::~MsgText(void)
{
}

void MsgText::setFile(const std::string &file, const int line)
{
	_file = file;
	_line = line;
}

std::string MsgText::format()
{
	std::time_t t = time(nullptr);
	struct tm *lt = localtime(&t);

	std::string strlog = "[";
	
	std::stringstream s;
	std::string  st;
	//年月日 时分秒
	s << lt->tm_year;
	s >> st;
	strlog.append(st);
	s.clear();

	s << lt->tm_mon;
	s >> st;
	strlog.append("-");
	strlog.append(st);
	s.clear();

	s << lt->tm_mday;
	s >> st;
	strlog.append("-");
	strlog.append(st);
	strlog.append(" ");
	s.clear();

	s << lt->tm_hour;
	s >> st;
	strlog.append(st);
	s.clear();

	s << lt->tm_min;
	s >> st;
	strlog.append(":");
	strlog.append(st);
	s.clear();

	s << lt->tm_sec;
	s >> st;
	strlog.append(":");
	strlog.append(st);
	s.clear();

	strlog.append("]");

	strlog.append("<<");
	strlog.append(_file);
	strlog.append("(");

	s << _line;
	s >> st;
	strlog.append(st);
	s.clear();
	strlog.append(")");
	strlog.append(">> ");

	return strlog;
}

std::string MsgText::format(const std::string &context)
{
std::time_t t = time(nullptr);
	struct tm *lt = localtime(&t);

	std::string strlog = "[";
	
	std::stringstream s;
	std::string  st;
	//年月日 时分秒
	s << lt->tm_year;
	s >> st;
	strlog.append(st);
	s.clear();

	s << lt->tm_mon;
	s >> st;
	strlog.append("-");
	strlog.append(st);
	s.clear();

	s << lt->tm_mday;
	s >> st;
	strlog.append("-");
	strlog.append(st);
	strlog.append(" ");
	s.clear();

	s << lt->tm_hour;
	s >> st;
	strlog.append(st);
	s.clear();

	s << lt->tm_min;
	s >> st;
	strlog.append(":");
	strlog.append(st);
	s.clear();

	s << lt->tm_sec;
	s >> st;
	strlog.append(":");
	strlog.append(st);
	s.clear();

	strlog.append("]");

	strlog.append(context);
	strlog.append("---");
	strlog.append(_file);
	strlog.append("(");

	s << _line;
	s >> st;
	strlog.append(st);
	s.clear();
	strlog.append(")");
	strlog.append("\n");

	return strlog;
}