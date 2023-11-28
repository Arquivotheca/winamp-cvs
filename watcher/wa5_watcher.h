#ifndef __WASABI_WA5_WATCHER_H
#define __WASABI_WA5_WATCHER_H

#include "../Winamp/api_wa5component.h"

class WA5_WATCHER : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};
#endif