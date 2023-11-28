/* copyright 2006 Ben Allison */
#ifndef __WASABI_WA5_ALAC_H
#define __WASABI_WA5_ALAC_H

#include "../Winamp/api_wa5component.h"

class WA5_ALAC : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};
#endif