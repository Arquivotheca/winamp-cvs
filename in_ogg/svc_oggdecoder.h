#pragma once
#include <bfc/dispatch.h>
#include <ogg/ogg.h>
#include "ifc_oggdecoder.h"
#include <api/service/services.h>

class NOVTABLE svc_oggdecoder : public Dispatchable
{
protected:
	svc_oggdecoder() {}
	~svc_oggdecoder() {}
public:
	static FOURCC getServiceType() { return WaSvc::OGGDECODER; } 
	ifc_oggdecoder *CreateDecoder(const ogg_packet *packet);

	enum
	{
		DISP_CREATEDECODER = 0,
	};
};

inline ifc_oggdecoder *svc_oggdecoder::CreateDecoder(const ogg_packet *packet)
{
	return _call(DISP_CREATEDECODER, (ifc_oggdecoder *)0, packet);
}