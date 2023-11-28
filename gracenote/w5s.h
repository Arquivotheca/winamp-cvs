#ifndef __WASABI_WA5_GRACENOTE_H_
#define __WASABI_WA5_GRACENOTE_H_

#include "../Winamp/api_wa5component.h"

class WA5_Gracenote : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

#endif