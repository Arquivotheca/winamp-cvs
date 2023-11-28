#pragma once
#include"syscb/ifc_syscallback.h"
#include "api_cloud.h"

class CloudCallback : public ifc_sysCallback
{
private:
	GUID WASABICALL SysCallback_GetEventType()
	{
		return api_cloud::GetSysCallbackGUID();
	}

	int WASABICALL SysCallback_Notify(int msg, intptr_t param1, intptr_t param2)
	{
		switch (msg)
		{
		case api_cloud::SYSCB_LOGGEDIN:
			CloudCallback_OnLoggedIn();
			return 1;
		
		default:
			return 0;
		}
	}
	
	virtual void CloudCallback_OnLoggedIn() {}

};