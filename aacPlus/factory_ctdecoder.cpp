#include "api.h"
#include "factory_ctdecoder.h"
#include "CTDecoder.h"

static const char serviceName[] = "Coding Technologies aacPlus Decoder";
static const char testString[] = "mp4a";

// {654B5212-018E-45f6-88CF-75862C85D99A}
static const GUID CTGUID = { 0x654b5212, 0x18e, 0x45f6, { 0x88, 0xcf, 0x75, 0x86, 0x2c, 0x85, 0xd9, 0x9a } };

FOURCC CTDecoderFactory::GetServiceType()
{
	return WaSvc::MP4AUDIODECODER; 
}

const char *CTDecoderFactory::GetServiceName()
{
	return serviceName;
}

GUID CTDecoderFactory::GetGUID()
{
	return CTGUID;
}

void *CTDecoderFactory::GetInterface(int global_lock)
{
	MP4AudioDecoder *ifc=new CTDecoder;
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int CTDecoderFactory::SupportNonLockingInterface()
{
	return 1;
}

int CTDecoderFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	MP4AudioDecoder *decoder = static_cast<MP4AudioDecoder *>(ifc);
	CTDecoder *aacPlusDecoder = static_cast<CTDecoder *>(decoder);
	delete aacPlusDecoder;
	return 1;
}

const char *CTDecoderFactory::GetTestString()
{
	return testString;
}

int CTDecoderFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS CTDecoderFactory
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
