#pragma once
#include "MutexLock.h"
#include "FileHandle.h"
#include <map>
#include <string>
#include <sstream>
#include "LoggerStream.h"

class Logger
{
public:
	typedef  std::map<std::string, Logger*>	 LoggerMap;
	typedef  std::map<std::string, Logger*>::iterator LoggerMapIter;

public:
	~Logger(void);

	static Logger* getLogger(const std::string);
	static void releseLogger(const std::string);

	Logger &log(const std::string &contxt, const char *file, const int line);
	Logger &log(const char *file, const int line);

	LoggerStream &logStream();

private:
	Logger(const std::string name);

	static MutexLock	_mutex;

	static LoggerMap	_loggerMap;

	FileHandle  _handle;

	LoggerStream _logStream;
};

//
//inline
//
inline LoggerStream &Logger::logStream()
{
	return _logStream;
}
//
// convenience macros
//

#define logger_log(logger, msg)\
	if(logger) (logger)->log((msg), (__FILE__), (__LINE__)); else (void) 0

#define LOGGER(logger)\
	if(logger) (logger)->log((__FILE__), (__LINE__)).logStream()

