#pragma once

#include <string>

class Exception
{
public:
	Exception(const std::string &name, const std::string &msg, int code = 0);
	////create exception with msg and code

	Exception(const std::string &msg);

	Exception(const Exception &exc);

	Exception operator = (const Exception &exc);

	Exception *clone() const;

	std::string displayText() const;
	
	int code() const;

	std::string message() const;

	std::string name() const;

private:
	std::string  _msg;
	std::string  _name;
	int			 _code;
};

//
//inlines
//
inline int Exception::code() const
{
	return _code;
}

inline std::string Exception::message() const
{
	return _msg;
}

inline std::string Exception::name() const
{
	return _name;
}