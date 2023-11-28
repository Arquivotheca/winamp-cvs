#include "./main.h"
#include "./plugin.h"
#include "./profile.h"
#include "./profileManager.h"
#include "./configManager.h"
#include "./filterPolicy.h"
#include "./formatData.h"
#include "./wasabiApi.h"
#include "./resource.h"

#include <strsafe.h>


#define PROFILE_FILENAME_PREFIX TEXT("dropboxProfile_")
#define PROFILE_SECTION TEXT("Profile")


static LPTSTR DuplicateString(LPCTSTR pszSource)
{
	if (NULL == pszSource)
		return NULL;

	INT cchBuffer = lstrlen(pszSource) + 1;
	LPTSTR pszBuffer = (LPTSTR)malloc(sizeof(TCHAR) * cchBuffer);
	if (NULL == pszBuffer)
		return NULL;
		
	if (FAILED(StringCchCopy(pszBuffer, cchBuffer, pszSource)))
	{
		free(pszBuffer);
		pszBuffer = NULL;
	}
	return pszBuffer;
}

Profile::Profile(const UUID &profileUid) :
	ref(1), uid(profileUid), filePath(NULL), name(NULL), description(NULL), filterPolicy(NULL)
{

}
Profile::~Profile()
{
	if (NULL != filePath)
		free(filePath);

	if (NULL != name && !IS_INTRESOURCE(name))
		lfh_free(name);

	if (NULL != description && !IS_INTRESOURCE(description))
		lfh_free(description);

	if (NULL != filterPolicy)
		filterPolicy->Release();
		
}

LPCTSTR Profile::GetFilePrefix()
{
	return PROFILE_FILENAME_PREFIX;
}

HRESULT Profile::MakeUniqueName(LPTSTR pszName, INT cchNameMax)
{
	Profile *szProfiles[256];
	INT profilesCount = PLUGIN_PROFILEMNGR->LoadProfiles(szProfiles, ARRAYSIZE(szProfiles));
	
	TCHAR szBuffer[512];
	LPTSTR *namesList= NULL;
	INT namesCount = 0;

	if (0 == profilesCount)
		return S_OK;
	
	namesList = (LPTSTR*)malloc(sizeof(LPTSTR) * profilesCount);
	if (NULL != namesList)
	{		
		INT cchBuffer;
		for (INT i = 0; i < profilesCount; i++)
		{
			if (SUCCEEDED(szProfiles[i]->GetName(szBuffer, ARRAYSIZE(szBuffer))))
			{
				cchBuffer = lstrlen(szBuffer);
				if (0 != cchBuffer)
				{
					cchBuffer++;
					namesList[namesCount] = (LPTSTR)malloc(sizeof(TCHAR) * cchBuffer);
					if (NULL != namesList[namesCount])
					{
						CopyMemory(namesList[namesCount], szBuffer, sizeof(TCHAR) * cchBuffer);
						namesCount++;
					}
				}
			}
		}
	}

	for (INT i = 0; i < profilesCount; i++)
	szProfiles[i]->Release();
	
	INT cchName = lstrlen(pszName);
	INT nameIndex = 1;
	BOOL repeatScan;
	
	HRESULT hr = S_OK;
	do
	{
		repeatScan = FALSE;
		for (INT i = 0; i < namesCount; i++)
		{
			if (CSTR_EQUAL == CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, namesList[i], -1, pszName, -1))
			{
				repeatScan = TRUE;
				break;
			}
		}

		if (repeatScan)
		{
			pszName[cchName] = TEXT('\0');
			StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT(" (%d)"), nameIndex++);
			hr = StringCchCat(pszName, cchNameMax, szBuffer);
			if (FAILED(hr))
			{
				pszName[cchName] = TEXT('\0');
				repeatScan = FALSE;
			}
		}

	} while(repeatScan);

	if (NULL != namesList)
	{
		for(INT i = 0; i < namesCount; i++)
			free(namesList[i]);
		free(namesList);
	}

	return hr;
}

Profile *Profile::CreateTemplate(const PROFILETEMPLATE *profileTemplate)
{
	if (NULL == profileTemplate)
		return NULL;

	Profile *instance = new Profile(profileTemplate->profileUid);
	if (NULL == instance)
		return NULL;

	instance->SetName(profileTemplate->name);
	instance->SetDescription(profileTemplate->description);
	instance->filterPolicy = FilterPolicy::Create();
	if (FAILED(instance->CreateFileName()) || FAILED(instance->Save()))
	{
		instance->Release();
		return NULL;
	}


	if (NULL != profileTemplate->configItems)
	{
		const CONFIGITEMTEMPLATE *itemTemplate;
		IConfiguration *pConfig = NULL;
		
		for (int i = 0; i < profileTemplate->itemsCount; i++)
		{
			itemTemplate = &profileTemplate->configItems[i];
			switch(itemTemplate->valueType)
			{
				case ConfigItemTypeSection: 
					if (FAILED(Plugin_QueryConfiguration(*itemTemplate->guidValue, instance, &pConfig)))
						pConfig = NULL;
					break;
				case ConfigItemTypeString: 
					if (NULL != pConfig)
						pConfig->WriteString(itemTemplate->valueName, itemTemplate->stringValue);
					break;
				case ConfigItemTypeInteger: 
					if (NULL != pConfig)
						pConfig->WriteInt(itemTemplate->valueName, itemTemplate->integerValue);
					break;
				case ConfigItemTypeFilterPolicy: 
					if (NULL != instance->filterPolicy && IS_INTRESOURCE(itemTemplate->valueName))
						instance->filterPolicy->SetRule((BYTE)(INT_PTR)itemTemplate->valueName, itemTemplate->integerValue);
					break;
			}
		}
	}

	if (NULL != instance->filterPolicy && 
		FAILED(instance->filterPolicy->Save(instance)))
	{
		instance->Delete();
		instance->Release();
		instance = NULL;
	}

	if (NULL != instance)
		instance->Notify(ProfileCallback::eventProfileCreated);
	
	return instance;
}

Profile *Profile::Create(HWND hwnd)
{
	TCHAR szName[256], szDescription[4096];
	
	WASABI_API_LNGSTRINGW_BUF(IDS_PROFILE_NEW, szName, ARRAYSIZE(szName));
	WASABI_API_LNGSTRINGW_BUF(IDS_PROFILE_NEW_DESCRIPTION, szDescription, ARRAYSIZE(szDescription));

	MakeUniqueName(szName, ARRAYSIZE(szName));

	Profile *instance = Profile::CreateEx(szName, szDescription);
	if (NULL == instance || FAILED(instance->Save()))
	{
		if (NULL != hwnd)
			MessageBox(hwnd, TEXT("Unable to create profile."), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	return instance;
}

Profile *Profile::CreateEx(LPCTSTR pszName, LPCTSTR pszDescription)
{
	UUID instanceUID;
	LONG result = UuidCreateSequential(&instanceUID);
	if (RPC_S_OK != result && RPC_S_UUID_LOCAL_ONLY != result && RPC_S_UUID_NO_ADDRESS != result)
		return NULL;

	Profile *instance = new Profile(instanceUID);
	if (NULL == instance)
		return NULL;

	instance->SetName(pszName);
	instance->SetDescription(pszDescription);
	instance->filterPolicy = FilterPolicy::Create();
	if (FAILED(instance->CreateFileName()) || FAILED(instance->Save()))
	{
		instance->Release();
		instance = NULL;
	}

	if (NULL != instance)
		instance->Notify(ProfileCallback::eventProfileCreated);

	return instance;
}
HRESULT Profile::WriteUID()
{
	RPC_WSTR pszProfileId;
	if (RPC_S_OK != UuidToString((UUID*)&uid, &pszProfileId))
		return E_UNEXPECTED;
	
	HRESULT result = S_OK;
	if (!WritePrivateProfileString(PROFILE_SECTION, TEXT("uid"), (LPCWSTR)pszProfileId, filePath))
	{
		DWORD errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
			result = HRESULT_FROM_WIN32(errorCode);
	}

	RpcStringFree(&pszProfileId);
	return result;
}

Profile *Profile::CreateCopy(Profile *source)
{
	if (NULL == source || NULL == source->filePath)
		return NULL;

	UUID instanceUID;
	LONG result = UuidCreateSequential(&instanceUID);
	if (RPC_S_OK != result && RPC_S_UUID_LOCAL_ONLY != result && RPC_S_UUID_NO_ADDRESS != result)
		return NULL;

	Profile *instance = new Profile(instanceUID);
	if (NULL == instance)
		return NULL;

	if (FAILED(instance->CreateFileName()))
	{
		instance->Release();
		return NULL;
	}

	source->Save();
	if (!CopyFile(source->filePath, instance->filePath, TRUE) ||
		FAILED(instance->WriteUID()))
	{
		instance->Delete();
		instance->Release();
		return NULL;
	}

	Profile *copy = Load(instance->filePath);
	instance->Release();
	
	if (NULL != copy)
	{
		TCHAR szBuffer[1024];
		WASABI_API_LNGSTRINGW_BUF(IDS_PROFILE_COPY_PREFIX, szBuffer, ARRAYSIZE(szBuffer));
		INT cchBuffer = lstrlen(szBuffer);
		if (0 != cchBuffer && TEXT(' ') != szBuffer[cchBuffer - 1])
		{
			if (cchBuffer < ARRAYSIZE(szBuffer))
			{
				szBuffer[cchBuffer] = TEXT(' ');
				szBuffer[++cchBuffer] = TEXT('\0');
			}
		}
		if (cchBuffer < ARRAYSIZE(szBuffer) &&
			SUCCEEDED(source->GetName(szBuffer + cchBuffer, ARRAYSIZE(szBuffer) - cchBuffer)))
		{
			MakeUniqueName(szBuffer, ARRAYSIZE(szBuffer));
			copy->SetName(szBuffer);
		}

		copy->Save();
		copy->Notify(ProfileCallback::eventProfileCreated);
	}
	
	return copy;
}
Profile *Profile::Load(LPCTSTR pszFilePath)
{
	TCHAR szBuffer[1024];

	GetPrivateProfileString(PROFILE_SECTION, TEXT("uid"), NULL, szBuffer, ARRAYSIZE(szBuffer), pszFilePath);
	
	UUID instanceUID;
	if (RPC_S_OK != UuidFromString((RPC_WSTR)szBuffer, &instanceUID))
		return NULL;

	Profile *instance = new Profile(instanceUID);
	if (NULL == instance)
		return NULL;
	
	instance->filePath = DuplicateString(pszFilePath);

	INT_PTR resourceId;
	LPCTSTR  pszValue;
	
	GetPrivateProfileString(PROFILE_SECTION, TEXT("name"), NULL, szBuffer, ARRAYSIZE(szBuffer), pszFilePath);
	pszValue = (ParseLangResource(szBuffer, &resourceId)) ? MAKEINTRESOURCE(resourceId) : szBuffer;
	instance->SetName(pszValue);
	
	GetPrivateProfileString(PROFILE_SECTION, TEXT("description"), NULL, szBuffer, ARRAYSIZE(szBuffer), pszFilePath);
	pszValue = (ParseLangResource(szBuffer, &resourceId)) ? MAKEINTRESOURCE(resourceId) : szBuffer;
	instance->SetDescription(pszValue);
	
	instance->filterPolicy = FilterPolicy::Load(instance);
	if (NULL == instance->filterPolicy)
	{
		instance->filterPolicy = FilterPolicy::Create();
		if (NULL != instance->filterPolicy)
			instance->filterPolicy->Save(instance);
	}

	return instance;
}



HRESULT Profile::FormatPath(const UUID *profileUid, LPCTSTR pszBasePath, LPTSTR pszBuffer, INT cchBufferMax)
{
	LPTSTR pszName = pszBuffer;
	size_t remaining = cchBufferMax;

	RPC_WSTR pszProfileId;
	if (RPC_S_OK != UuidToString((UUID*)profileUid, &pszProfileId))
		return E_FAIL;

	HRESULT hr = S_OK;
	
	if (NULL != pszBasePath)
	{
		INT cchPath = lstrlen(pszBasePath);
		while(cchPath > 0 && TEXT('\\') == pszBasePath[cchPath - 1]) cchPath--;
		while(cchPath > 0 && TEXT('/') == pszBasePath[cchPath - 1]) 	cchPath--;

		hr = StringCchCopyNEx(pszName, remaining, pszBasePath, cchPath, &pszName, &remaining, 0);
	}
	 
	if (SUCCEEDED(hr))
		hr = StringCchPrintf(pszName, remaining, TEXT("\\") PROFILE_FILENAME_PREFIX TEXT("{%s}.ini"), pszProfileId);
	
	RpcStringFree(&pszProfileId);

	return hr;
}
ULONG Profile::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG Profile::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT Profile::GetUID(UUID *puid)
{
	if (NULL == puid)
		return E_POINTER;
	CopyMemory(puid, &uid, sizeof(UUID));
	return S_OK;
}

HRESULT Profile::GetName(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;

	if (NULL == name) 
		*pszBuffer = TEXT('\0');
	else if (IS_INTRESOURCE(name))
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)name, pszBuffer, cchBufferMax);	
	else
		hr = StringCchCopy(pszBuffer, cchBufferMax, name);
	
	return hr;
}

HRESULT Profile::GetDescription(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = S_OK;

	if (NULL == description) 
		*pszBuffer = TEXT('\0');
	else if (IS_INTRESOURCE(description))
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)description, pszBuffer, cchBufferMax);	
	else
		hr = StringCchCopy(pszBuffer, cchBufferMax, description);
	
	return hr;
}


HRESULT Profile::CreateFileName()
{
	if (NULL != filePath)
	{
		free(filePath);
		filePath = NULL;
	}

	TCHAR szPath[MAX_PATH], szFile[MAX_PATH * 2];
	HRESULT hr = Plugin_GetDropboxPath(szPath, ARRAYSIZE(szPath));
	if (FAILED(hr))
		return E_OUTOFMEMORY;

	hr = FormatPath(&uid, szPath, szFile, ARRAYSIZE(szFile));
	if (FAILED(hr))
		return hr;

	filePath = DuplicateString(szFile);
	if (NULL == filePath)
		hr = E_OUTOFMEMORY;

	Plugin_EnsurePathExist(szPath);

	return hr;
}

HRESULT Profile::GetFilePath(LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == filePath) return E_UNEXPECTED;
	return StringCchCopy(pszBuffer, cchBufferMax, filePath);
}

HRESULT Profile::SetName(LPCTSTR  pszName)
{
	HRESULT hr = S_OK;

	if (NULL != name)
	{
		if (!IS_INTRESOURCE(name)) lfh_free(name);
		name = NULL;	
	}
		
	if (NULL != pszName)
	{
		name = (IS_INTRESOURCE(pszName)) ? (LPTSTR)pszName : lfh_strdup(pszName);
		if (NULL == name) 
			hr = E_OUTOFMEMORY;
	}
	return hr;
}

HRESULT Profile::SetDescription(LPCTSTR  pszDescription)
{
	HRESULT hr = S_OK;

	if (NULL != description)
	{
		if (!IS_INTRESOURCE(description)) lfh_free(description);
		description = NULL;	
	}
		
	if (NULL != pszDescription)
	{
		description = (IS_INTRESOURCE(pszDescription)) ? (LPTSTR)pszDescription : lfh_strdup(pszDescription);
		if (NULL == description) 
			hr = E_OUTOFMEMORY;
	}
	return hr;
}


HRESULT Profile::Save()
{	
	if (NULL == filePath) return E_UNEXPECTED;
	
	TCHAR szBuffer[2048];
	LPCTSTR pszValue;

	HRESULT hr;

	hr = WriteUID();
	if (FAILED(hr)) return hr;

	pszValue = IS_INTRESOURCE(name) ? FormatLangResource((INT_PTR)name, szBuffer,ARRAYSIZE(szBuffer)) : name;
	WritePrivateProfileString(PROFILE_SECTION, TEXT("name"), pszValue , filePath);
	
	pszValue = IS_INTRESOURCE(description) ? FormatLangResource((INT_PTR)description, szBuffer,ARRAYSIZE(szBuffer)) : description;
	WritePrivateProfileString(PROFILE_SECTION, TEXT("description"), pszValue, filePath);
			
	Notify(ProfileCallback::eventNameChanged);

	return S_OK;
}

HRESULT Profile::Delete()
{
	if (NULL == filePath) return S_OK;
	
	DWORD error = ERROR_SUCCESS;
	if (!DeleteFile(filePath))
	{
		error = GetLastError();
		if (ERROR_FILE_NOT_FOUND == error)
			error = ERROR_SUCCESS;
	}

	if (ERROR_SUCCESS != error)
		return HRESULT_FROM_WIN32(error);
	
	if (NULL != filePath)
	{
		free(filePath);
		filePath = NULL;
	}

	Notify(ProfileCallback::eventProfileDeleted);
	
	return S_OK;
}

void Profile::Notify(UINT eventId)
{
	PLUGIN_PROFILEMNGR->Notify(uid, eventId);
}

HRESULT Profile::QueryConfiguration(REFGUID configId, IConfiguration **ppConfig)
{
	return PLUGIN_CFGMNGR->QueryConfiguration(configId, this, ppConfig);
}

HRESULT Profile::GetFilterPolicy(FilterPolicy **ppPolicy, BOOL forceReload)
{
	if (NULL == ppPolicy)
        return E_POINTER;

	if (forceReload)
	{
		FilterPolicy *copy = FilterPolicy::Load(this);
		if (NULL != copy)
		{
			if (NULL != filterPolicy)
				filterPolicy->Release();
			filterPolicy = copy;
		}
	}
	
	*ppPolicy = filterPolicy;
	if (NULL == filterPolicy)
		return E_UNEXPECTED;
	
	filterPolicy->AddRef();
	return S_OK;
}
