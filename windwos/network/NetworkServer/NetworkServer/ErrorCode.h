#pragma once

namespace Net_Com{
	enum{
		NS_ERR_OK = 0,
		NS_ERR_CREATE_IOCP = 1,
		NS_ERR_BIND_IOCP,
		NS_ERR_BIND,
		NS_ERR_LISTEN,
		NS_ERR_GETFN,
		NS_ERR_POST_ACCEPT,
		NS_ERR_ACCEPT,
	};

	typedef struct _MsgData{
		int		nMsgType;
		int     nMsgCmd;
		void	*cMsgContent;
		int     nMsgSize;

		_MsgData()
		{
			nMsgType = 0;
			nMsgCmd = 0;
			cMsgContent = nullptr;
			nMsgSize = 0;
		}
	}MsgData;

	typedef int(*pMsgCallBack)(const MsgData &msg, void *pContext);

	class CAbsMsgCallback{
	public:
		CAbsMsgCallback(void *pContext);
		virtual ~CAbsMsgCallback();
		virtual long notify_msg(const MsgData &msg, void *pContext) = 0;

	protected:
		void *m_pContext;
	};
}