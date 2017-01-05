#pragma once
#include "MutexLock.h"
#include "FileHandle.h"

class Logger
{
public:
	~Logger(void);

	static Logger* getLogger(const char *name);

	void log(const char *contxt, const char *file, const int line);

private:
	Logger(const char *name = nullptr);

	static Logger*		_pLoger;

	static MutexLock	_mutex;

	FileHandle  _handle;
};

//
// convenience macros
//

#define logger_log(logger, msg)\
	if(logger) (logger)->log((msg), (__FILE__), (__LINE__)); else (void) 0

