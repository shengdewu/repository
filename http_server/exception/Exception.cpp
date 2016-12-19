#include "Exception.h"


Exception::Exception(const std::string &name, const std::string &msg, int code):
	_msg(msg),
	_name(name),
	_code(code)
{
}

Exception::Exception(const std::string &msg):
	_msg(msg),
	_name(""),
	_code(0)
{
}

Exception::Exception(const Exception &exc)
{
	if(this != &exc)
	{
		this->_msg = exc._msg;
		this->_code = exc._code;
	}
}

Exception Exception::operator=(const Exception &exc)
{
	if(this != &exc)
	{
		this->_msg = exc._msg;
		this->_code = exc._code;		
	}

	return *this;
}

Exception *Exception::clone() const
{
	return new Exception(*this);
}

std::string Exception::displayText() const
{
	std::string txt = name();

	if(!_msg.empty())
	{
		txt.append(":");
		txt.append(_msg);
	}

	return txt;
}
