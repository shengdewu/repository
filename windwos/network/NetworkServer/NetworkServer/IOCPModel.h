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
	*@brief ��ʼ�������,�ڿ�ʼ�����ʼ��֮ǰ������ã����캯�����Ѿ�����
	*@param [in] 
	*@return [long]
				���ñ�־���ɹ�����true������false
	*/
	bool LoadSocketLib();

	/*!
	*@name 
	*@brief �ͷ�����⣬�ڹر��������Ӻ��ڵ��ã������������Ѿ�����
	*@param [in] 
	*@return [long]
	*/
	void UnloadSocketLib();
};

