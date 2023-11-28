#ifndef __WASABI_WA5_BMP_H
#define __WASABI_WA5_BMP_H

#include "../Winamp/api_wa5component.h"

class WA5_BMP : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};
#endif