#pragma once

#include "../Agave/URIHandler/svc_urihandler.h"

// {A31A2453-5B08-45fe-B414-175A655E92E4}
static const GUID ml_addons_uri_handler = 
{ 0xa31a2453, 0x5b08, 0x45fe, { 0xb4, 0x14, 0x17, 0x5a, 0x65, 0x5e, 0x92, 0xe4 } };

class AddonsURIHandler : public svc_urihandler
{
public:
	static const char *getServiceName() { return "Winamp Add-ons URI Handler"; }
	static GUID getServiceGuid() { return ml_addons_uri_handler; } 
	int ProcessFilename(const wchar_t *filename);
	int IsMine(const wchar_t *filename); // just like ProcessFilename but don't actually process

protected:
	RECVS_DISPATCH;
};