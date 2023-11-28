#pragma once
#include <api/syscb/callbacks/svccb.h>
#include <api/syscb/api_syscb.h>
//#include "../replicant/cloud/api_cloud.h"

enum 
{
	// System callbacks for api_cloud
	SYSCALLBACK = MK4CC('c','l','o','d'),	// Unique identifier for mldb_api callbacks
	CLOUD_UPLOAD_START = 10,				// param1 = filename, param2 = (not used), Callback event for when a new file is added to the local mldb
	CLOUD_UPLOAD_DONE = 20,					// param1 = filename, param2 = (not used), Callback event for when a file is removed from the local mldb (before it happens)
};

class CloudCallback : public SysCallback
{
private:
	FOURCC GetEventType()
	{
		return MK4CC('c','l','o','d')/*api_cloud::SYSCALLBACK*/;
	}

	int notify(int msg, intptr_t param1, intptr_t param2)
	{
		const wchar_t *filename = (const wchar_t *)param1;

		switch (msg)
		{
			case CLOUD_UPLOAD_START:
				OnCloudUploadStart(filename);
			return 1;

			case CLOUD_UPLOAD_DONE:
				OnCloudUploadDone(filename, param2);
			return 1;

			default:
			return 0;
		}
	}
	virtual void OnCloudUploadStart(const wchar_t *filename) {}
	virtual void OnCloudUploadDone(const wchar_t *filename, int code) {}

#define CBCLASS CloudCallback
	START_DISPATCH_INLINE;
	CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
	CB(SYSCALLBACK_NOTIFY, notify);
	END_DISPATCH;
#undef CBCLASS
};