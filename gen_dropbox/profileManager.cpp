#include "./main.h"
#include "./plugin.h"
#include "./profileManager.h"
#include "./wasabiApi.h"

#include <shlwapi.h>
#include <strsafe.h>

#define PROFILE_PATH	TEXT("\\Plugins\\dropBox")



ProfileManager::ProfileManager() : cachedCallbackIndex(0)
{
}

ProfileManager::~ProfileManager()
{
	CallbackEntry *entry = NULL;

	for(size_t i = 0; i < globalCallbacks.size(); i++)
	{
		globalCallbacks[i]->Release();
	}
		
	

	for (size_t i = 0; i < registeredCallbacks.size(); i++)
	{
		entry = &registeredCallbacks[i];
		if (NULL != entry->callbackList)
		{
			for(size_t k = 0; k < entry->callbackList->size(); k++)
			{			
				if (NULL != entry->callbackList->at(i))
					entry->callbackList->at(i)->Release();
			}
			delete(entry->callbackList);
		}
	}
}


Profile *ProfileManager::LoadProfile(const UUID &profileUid)
{
	TCHAR szPath[MAX_PATH], szProfile[MAX_PATH * 2];
	HRESULT hr;
	hr = Plugin_GetDropboxPath(szPath, ARRAYSIZE(szPath));
	if (SUCCEEDED(hr))
		hr = Profile::FormatPath(&profileUid, szPath, szProfile, ARRAYSIZE(szProfile));
	
	if (FAILED(hr))
		return NULL;

	return Profile::Load(szProfile);
}

INT ProfileManager::LoadProfiles(Profile **pszProfiles, INT cchProfilesMax)
{
	
	TCHAR szPath[MAX_PATH], szFile[MAX_PATH * 2];
	HRESULT hr;

	hr = Plugin_GetDropboxPath(szPath, ARRAYSIZE(szPath));
	if (SUCCEEDED(hr))
	{
		TCHAR szFilter[MAX_PATH];
		hr = StringCchPrintf(szFilter, ARRAYSIZE(szFilter), 	TEXT("%s*.ini"), Profile::GetFilePrefix());
		PathCombine(szFile, szPath, szFilter);
	}

	if (FAILED(hr))
		return 0;

	WIN32_FIND_DATA findData;
	HANDLE hFind = FindFirstFile(szFile, &findData);
	if (INVALID_HANDLE_VALUE == hFind)
		return 0;

	INT profileLoaded = 0;
	Profile *profile;
	
	if(NULL == pszProfiles)
		cchProfilesMax = 0x0FFFFFFF;

	do
	{
		PathCombine(szFile, szPath, findData.cFileName);
		
		profile = Profile::Load(szFile);
		if (NULL != profile)
		{			
			if (NULL == pszProfiles)
			{
				profile->Release();
			}
			else
			{
				pszProfiles[profileLoaded] = profile;
			}
			profileLoaded++;
		}
			
	} while(FindNextFile(hFind, &findData) && profileLoaded < cchProfilesMax);
	FindClose(hFind);

	return profileLoaded;
}

BOOL ProfileManager::RegisterCallback(const UUID &profileUid, ProfileCallback *callback)
{
	if (NULL == callback)
		return FALSE;
	
	if (IsEqualGUID(GUID_NULL, profileUid))
	{
		for(size_t i = 0; i < globalCallbacks.size(); i++)
		{
			if (callback == globalCallbacks[i])
				return TRUE;
		}
		globalCallbacks.push_back(callback);
		callback->AddRef();
		return TRUE;
	}

	CallbackEntry *entry = NULL;
	for (size_t i = 0; i < registeredCallbacks.size(); i++)
	{
		if (IsEqualGUID(profileUid, registeredCallbacks[i].profileUid))
		{
			entry = &registeredCallbacks[i];
			break;
		}
	}

	if (NULL == entry)
	{
		CallbackEntry newEntry;
		newEntry.profileUid = profileUid;
		newEntry.callbackList = new CallbackList();
		registeredCallbacks.push_back(newEntry);
		entry = &registeredCallbacks.back();
	}

	if (NULL == entry || NULL == entry->callbackList)
		return FALSE;
	
	for(size_t i = 0; i < entry->callbackList->size(); i++)
	{
		if (callback == entry->callbackList->at(i))
			return TRUE;
	}
	entry->callbackList->push_back(callback);
	callback->AddRef();

	return TRUE;
}

BOOL ProfileManager::UnregisterCallback(const UUID &profileUid, ProfileCallback *callback)
{
	if (NULL == callback)
		return FALSE;
	
	if (IsEqualGUID(GUID_NULL, profileUid))
	{
		for(size_t i = 0; i < globalCallbacks.size(); i++)
		{
			if (callback == globalCallbacks[i])
			{
				globalCallbacks.eraseindex(i);
				callback->Release();
				return TRUE;
			}
		}
		return FALSE;
	}

	CallbackEntry *entry = NULL;
	for (size_t i = 0; i < registeredCallbacks.size(); i++)
	{
		if (IsEqualGUID(profileUid, registeredCallbacks[i].profileUid))
		{
			entry = &registeredCallbacks[i];
			for(size_t k = 0; k < entry->callbackList->size(); k++)
			{
				if (callback == entry->callbackList->at(k))
				{
					entry->callbackList->eraseindex(k);
					callback->Release();
					if (0 == entry->callbackList->size())
					{
						delete(entry->callbackList);
						 registeredCallbacks.eraseAt(i);
					}
					return TRUE;
				}
			}
			break;
		}
	}
	return FALSE;
}

void ProfileManager::Notify(const UUID &profileUid, UINT eventId)
{	
	CallbackEntry *entry = NULL;

	for (size_t i = 0; i < globalCallbacks.size(); i++)
	{
		globalCallbacks[i]->Notify(eventId, &profileUid);
	}

	if (0 != cachedCallbackIndex && 
		cachedCallbackIndex < registeredCallbacks.size() && 
		IsEqualGUID(profileUid, registeredCallbacks[cachedCallbackIndex].profileUid))
	{
		entry = &registeredCallbacks[cachedCallbackIndex];
	}
	else
	{
		for (size_t i = 0; i < registeredCallbacks.size(); i++)
		{
			if (IsEqualGUID(profileUid, registeredCallbacks[i].profileUid))
			{
				entry = &registeredCallbacks[i];
				cachedCallbackIndex = i;
				break;
			}
		}
	}

	if (NULL != entry && NULL != entry->callbackList)
	{
		for(size_t k = 0; k < entry->callbackList->size(); k++)
		{					
			entry->callbackList->at(k)->Notify(eventId, &profileUid);
		}
	}
}
