#pragma once
#include <string>

class FileHandle;
class LoggerStream
{
public:
	LoggerStream(void);
	~LoggerStream(void);

	LoggerStream &operator << (const std::string &contxt);
	LoggerStream &operator << (const long	contxt);
	void setHandle(FileHandle* pHandle);

private:
	FileHandle*  _pHandle;
};

