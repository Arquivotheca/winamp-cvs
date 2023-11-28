#pragma once
#include "foundation/dispatch.h"
#include "service/types.h"
#ifdef __ANDROID__
#include <jni.h>
#endif

// {D71A5206-50E4-4CE1-ADE7-5D5378852E5A}
static const GUID androidApiServiceGUID = 
{ 0xd71a5206, 0x50e4, 0x4ce1, { 0xad, 0xe7, 0x5d, 0x53, 0x78, 0x85, 0x2e, 0x5a } };


// ----------------------------------------------------------------------------
class NOVTABLE api_android : public Wasabi2::Dispatchable
{
protected:
	api_android()	: Dispatchable(DISPATCHABLE_VERSION) {}
	~api_android()	{}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return androidApiServiceGUID; }

	int GetSDKVersion() { return Android_GetSDKVersion(); } // SDK level, e.g. 9
#ifdef __ANDROID__
	JavaVM *GetJVM() { return Android_GetJVM(); }
#endif
int GetRelease(nx_string_t *release) { return Android_GetRelease(release); } // Release version number, e.g. "2.3.1"


	enum
	{
		DISPATCHABLE_VERSION,
	};

protected:
	virtual int WASABICALL Android_GetSDKVersion()=0;
#ifdef __ANDROID__
	virtual JavaVM *WASABICALL Android_GetJVM()=0;
#endif
	virtual int WASABICALL Android_GetRelease(nx_string_t *release)=0;
};

