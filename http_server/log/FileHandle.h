#pragma once

class FileHandle
{
public:
	FileHandle(const char *pFile = nullptr);
	~FileHandle(void);

	long write(const char *pMsg, const int size);
	void open(const char *pFile = nullptr);
	void close();

private:
	int	 _file;
};

