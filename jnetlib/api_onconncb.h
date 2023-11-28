#ifndef __WASABI_API_WEBSERV_ONCONNCB_H
#define __WASABI_API_WEBSERV_ONCONNCB_H
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
//#include "listen.h"

class api_webserv;
class api_pagegenerator;
class api_httpserv;
class JNL_Listen;
class api_onconncb : public Dispatchable
{
public:
	api_pagegenerator* onConnection(api_httpserv *serv, int port);
	//api_pagegenerator* onConnection2(api_httpserv *serv, JNL_Listen *listener);
	void destroyConnection(api_pagegenerator *conn);
	DISPATCH_CODES  
	{
		API_ONCONNCB_ONCONNECTION = 10,
		//API_ONCONNCB_ONCONNECTION2 = 15,
		API_ONCONNCB_DESTROYCONNECTION = 20,
	};
	api_webserv *caller;
};
inline api_pagegenerator* api_onconncb::onConnection(api_httpserv *serv, int port)
{
	return _call(API_ONCONNCB_ONCONNECTION, (api_pagegenerator *)0, serv, port);
}
/*
inline api_pagegenerator* api_onconncb::onConnection2(api_httpserv *serv, JNL_Listen *listener)
{
	void *params[2] = { &serv, &listener};
	api_pagegenerator *retval;

	if (_dispatch(API_ONCONNCB_ONCONNECTION2, &retval, params, 2))
		return retval;
	else
	{
		return onConnection(serv, listener->port());
	}	
}*/

inline void api_onconncb::destroyConnection(api_pagegenerator *connection)
{
	_voidcall(API_ONCONNCB_DESTROYCONNECTION, connection);
}
#endif