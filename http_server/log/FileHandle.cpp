#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "FileHandle.h"
#include "LogDef.h"
#include "Exception.h"

FileHandle::FileHandle(const char *pFile):
	_file(LOG_INVALID_FILE)
{
	if(pFile)
	{
		_file = ::open(pFile, O_RDWR | O_CREAT |O_APPEND);
		if(LOG_INVALID_FILE == _file)
		{
			std::string log = pFile;
			throw Exception(log.append(": create it, but it is failed"));
		}
	}
}


FileHandle::~FileHandle(void)
{
	close();	
}

long FileHandle::write(const char *pMsg, const int size)
{
	if(LOG_INVALID_FILE == _file)
	{
		return LOG_FILE_ERROR;
	}

	long nwrite = ::write(_file, pMsg, size);
	while(nwrite < size)
	{
		nwrite += ::write(_file, pMsg + nwrite, size - nwrite);
	}

	return nwrite;
}

void FileHandle::open(const char *pFile)
{
	if(LOG_INVALID_FILE != _file)
	{
		return;
	}

	_file = ::open(pFile, O_RDWR | O_CREAT |O_APPEND);
	if(LOG_INVALID_FILE == _file)
	{
		std::string log = pFile;
		throw Exception(log.append(": create it, but it is failed"));
	}
}

void FileHandle::close()
{
	if(LOG_INVALID_FILE != _file)
	{
		::close(_file);;
	}
}
