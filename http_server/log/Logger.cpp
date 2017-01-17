#include "Logger.h"
#include "MsgText.h"
#include "LogDef.h"
#include "ScopedMutexLock.h"

Logger::LoggerMap Logger::_loggerMap;
MutexLock Logger::_mutex;

Logger::Logger(const std::string name)
{
	std::string strLog = name;
	if(strLog.empty())
	{
		strLog = LOG_DEFAULT_NAME;
	}
	_handle.open(strLog.c_str());
	_logStream.setHandle(&_handle);
}

Logger::~Logger(void)
{
	_handle.close();
}

Logger* Logger::getLogger(const std::string name)
{
	ScopedMutexLock lock(&_mutex);
	LoggerMapIter  iter = _loggerMap.find(name);
	if(iter != _loggerMap.end())
	{
		return iter->second;
	}

	Logger *pLogger = new Logger(name);
	_loggerMap[name] = pLogger;

	return pLogger;
}

void Logger::releseLogger(const std::string name)
{
	ScopedMutexLock lock(&_mutex);
	LoggerMapIter  iter = _loggerMap.find(name);
	if(iter != _loggerMap.end())
	{
		delete iter->second;
		iter->second = nullptr;
		_loggerMap.erase(iter);
	}
}

Logger& Logger::log(const std::string &contxt, const char *file, const int line)
{
	MsgText msgText;
	msgText.setFile(file, line);
	
	std::string txt = msgText.format(contxt);

	_handle.write(txt.c_str(), txt.size());

	return *this;
}

Logger& Logger::log(const char *file, const int line)
{
	MsgText msgText;
	msgText.setFile(file, line);
	
	_logStream<<msgText.format();

	return *this;
}

