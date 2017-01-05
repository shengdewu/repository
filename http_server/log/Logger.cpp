#include "Logger.h"
#include "MsgText.h"

Logger* Logger::_pLoger = nullptr;
MutexLock Logger::_mutex;

Logger::Logger(const char *name)
{
	_handle.open(name);
}

Logger::~Logger(void)
{
	_handle.close();
}

Logger* Logger::getLogger(const char *name)
{
	if(nullptr == _pLoger)
	{
		_mutex.lock();
		if(nullptr == _pLoger)
		{
			_pLoger = new Logger(name);
		}
	}

	return _pLoger;
}

void Logger::log(const char *contxt, const char *file, const int line)
{
	MsgText msgText;
	msgText.setFile(file, line, contxt);
	
	std::string txt = msgText.format();

	_handle.write(txt.c_str(), txt.size());
}

