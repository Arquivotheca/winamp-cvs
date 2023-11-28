#include "./main.h"
#include "./dropWindow.h"
#include "./dropWindowInternal.h"
#include "./wasabiApi.h"
#include "./profile.h"
#include "./profileManager.h"



#include <shlwapi.h>
#include <strsafe.h>


#define DROPWNDPROFILE_PROP			TEXT("waDropWindowProfile")

class DropWindowProfile : public ProfileCallback
{

protected:
	DropWindowProfile(HWND hwndHost, Profile *activeProfile) 
		: ref(1), hwnd(hwndHost), profile(activeProfile)
	{
		if (NULL != profile)
		{
			profile->AddRef();
		}
	}

	virtual ~DropWindowProfile() 
	{
		if (NULL != profile)
		{
			profile->Release();
		}
		RemoveProp(hwnd, DROPWNDPROFILE_PROP);
	}

public:
	static BOOL RegisterProfile(HWND hwnd, Profile *profile)
	{
		if (NULL == hwnd) return FALSE;

		UUID profileUid;
		DropWindowProfile *instance = GetInstance(hwnd);
		if (NULL != instance)
		{			
			if (NULL != instance->profile &&
				SUCCEEDED(instance->profile->GetUID(&profileUid)))
			{
				PLUGIN_PROFILEMNGR->UnregisterCallback(profileUid, instance);
			}

			instance->Release();
			instance = NULL;
		}

		if (NULL == profile)
			return TRUE;

		
		instance = new DropWindowProfile(hwnd, profile);
		if (NULL == instance || !SetProp(hwnd, DROPWNDPROFILE_PROP, (HANDLE)instance))
		{
			if (NULL != instance)
			{
				instance->Release();
				instance = NULL;
			}
			return FALSE;
		}
			
		if (NULL != instance &&
			NULL != instance->profile &&
			SUCCEEDED(instance->profile->GetUID(&profileUid)))
		{
			PLUGIN_PROFILEMNGR->RegisterCallback(profileUid, instance);
		}

		return TRUE;
	}

	static DropWindowProfile *GetInstance(HWND hwnd)
	{
		return (DropWindowProfile*)GetProp(hwnd, DROPWNDPROFILE_PROP);
	}

	ULONG AddRef(void) { return InterlockedIncrement((LONG*)&ref); }
	ULONG Release(void) 
	{
		if (0 == ref)
			return ref;
	
		LONG r = InterlockedDecrement((LONG*)&ref);
		if (0 == r)
			delete(this);
		return r;
	}
	
	void Notify(UINT eventId, const UUID *profileUid) 
	{
		DropboxWindow_ProfileNotify(hwnd, eventId, profileUid);
	}

protected:
	ULONG ref;
	HWND hwnd;
	Profile *profile;

};

BOOL DropboxWindow_RegisterProfileCallback(HWND hwnd, Profile *profile)
{
	return DropWindowProfile::RegisterProfile(hwnd, profile);
}
BOOL DropboxWindow_UnregisterProfileCallback(HWND hwnd)
{
	return DropWindowProfile::RegisterProfile(hwnd, NULL);
}