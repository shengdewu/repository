#include "LoggerStream.h"
#include "FileHandle.h"
#include <sstream>

LoggerStream::LoggerStream(void):
	_pHandle(nullptr)
{
}


LoggerStream::~LoggerStream(void)
{
}


LoggerStream & LoggerStream::operator << (const std::string &contxt)
{
	_pHandle->write(contxt.c_str(), contxt.size());
	return *this;
}

LoggerStream & LoggerStream::operator << (const long contxt)
{
	std::string strtmp;
	std::stringstream s;
	s << contxt;
	s >> strtmp;

	_pHandle->write(strtmp.c_str(), strtmp.size());
	return *this;
}


void LoggerStream::setHandle(FileHandle* pHandle)
{
	_pHandle = pHandle;
}
