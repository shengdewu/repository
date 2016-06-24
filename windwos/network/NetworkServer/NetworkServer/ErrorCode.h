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
	};

}