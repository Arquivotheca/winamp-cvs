#include "LoggerFactory.h"
#include "Logger.h"
#include <api/service/service.h>

/*
 This is the GUID for our service factory
 don't re-use this.  
 make your own guid with guidgen.exe
 lives somewhere like C:\program files\Microsoft Visual Studio .NET 2003\Common7\Tools\Bin
*/

// {7AFCBA48-B071-4664-A222-FBFE42A1C7AF}
static const GUID LoggerGUID = 
{ 0x7afcba48, 0xb071, 0x4664, { 0xa2, 0x22, 0xfb, 0xfe, 0x42, 0xa1, 0xc7, 0xaf } };

// our Logger object.
static Logger logger;

FOURCC LoggerFactory::GetServiceType()
{
    return WaSvc::UNIQUE;
}

const char *LoggerFactory::GetServiceName()
{
	return "Winamp Logging Service";
}

GUID LoggerFactory::GetGuid()
{
	return LoggerGUID;
}

void *LoggerFactory::GetInterface(int global_lock)
{
    // logger is a singleton object, so we can just return a pointer to it
	// depending on what kind of service you are making, you might have to 
	// 'new' an object and return that instead (and then free it in ReleaseInterface)
	return &logger;
}

int LoggerFactory::ReleaseInterface(void *ifc)
{
	// no-op because we returned a singleton (see above)
	return 1;
}

// Define the dispatch table
#define CBCLASS LoggerFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName) 
CB(WASERVICEFACTORY_GETGUID, GetGuid)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
END_DISPATCH;
#undef CBCLASS