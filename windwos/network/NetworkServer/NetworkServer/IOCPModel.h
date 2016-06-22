#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")

class CIOCPModel
{
public:
	CIOCPModel(void);
	~CIOCPModel(void);

private:
	/*!
	*@name 
	*@brief 初始化网络库,在开始网络初始化之前必须调用，构造函数里已经调用
	*@param [in] 
	*@return [long]
				设置标志，成功返回true，否则false
	*/
	bool LoadSocketLib();

	/*!
	*@name 
	*@brief 释放网络库，在关闭所有连接后在调用，析构函数里已经调用
	*@param [in] 
	*@return [long]
	*/
	void UnloadSocketLib();
};

