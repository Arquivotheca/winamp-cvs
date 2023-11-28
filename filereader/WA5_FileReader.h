#ifndef __WASABI_WA5_FILEREADER_H
#define __WASABI_WA5_FILEREADER_H

#include "../Winamp/api_wa5component.h"

class WA5_FileReader : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};
#endif