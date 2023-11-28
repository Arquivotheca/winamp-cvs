//#define GUID_EQUALS_DEFINED
#include "api.h"
#include "factory_aacplusdecoder.h"
#include "impl_aacplusdecoder.h"

static const char serviceName[] = "Coding Technologies AAC+ Decoder";

FOURCC AacPlusDecoderFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *AacPlusDecoderFactory::GetServiceName()
{
	return serviceName;
}

GUID AacPlusDecoderFactory::GetGUID()
{
	return aacPlusDecoderGUID;
}

void *AacPlusDecoderFactory::GetInterface(int global_lock)
{
	api_aacplusdecoder *ifc=new AacPlusDecoder;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int AacPlusDecoderFactory::SupportNonLockingInterface()
{
	return 1;
}

int AacPlusDecoderFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_aacplusdecoder *decoder = static_cast<api_aacplusdecoder *>(ifc);
	AacPlusDecoder *aacPlusDecoder = static_cast<AacPlusDecoder *>(decoder);
	delete aacPlusDecoder;
	return 1;
}

const char *AacPlusDecoderFactory::GetTestString()
{
	return NULL;
}

int AacPlusDecoderFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS AacPlusDecoderFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
