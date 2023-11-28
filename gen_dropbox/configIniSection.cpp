#include "./main.h"
#include "./plugin.h"
#include "./configIniSection.h"
#include "./wasabiApi.h"
#include "./profile.h"
#include <shlwapi.h>
#include <strsafe.h>


#define PLUGININI_NAME		TEXT("\\dropbox.ini")
#define MEDIALIB_INI		TEXT("\\Plugins\\gen_ml.ini")

#define KEYLOOKUP(__key, __testString, __keyOut)\
	{if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, (__key), -1, (__testString), -1)){\
		__keyOut = (__key); return S_OK;}}

static LPTSTR GetPluginIniPath(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = Plugin_GetDropboxPath(pszBuffer, cchBufferMax);
	if (FAILED(hr))
		return NULL;

	INT pos = lstrlen(pszBuffer);
	if (cchBufferMax <= pos)
		return NULL;

	Plugin_EnsurePathExist(pszBuffer);

	hr = StringCchCopy(pszBuffer + pos, cchBufferMax - pos, PLUGININI_NAME);
	
	if (FAILED(hr))
	{
		*pszBuffer = TEXT('\0');
		return NULL;
	}
	
	return pszBuffer;
}

static LPTSTR GetMediaLibraryIniPath(LPTSTR pszBuffer, INT cchBufferMax)
{
	HRESULT hr = Plugin_GetWinampIniDirectory(pszBuffer, cchBufferMax);
	if (FAILED(hr))
		return NULL;

	INT pos = lstrlen(pszBuffer);
	hr = (cchBufferMax > pos) ? 
			StringCchCopy(pszBuffer + pos, cchBufferMax - pos, MEDIALIB_INI) : 
			STRSAFE_E_INSUFFICIENT_BUFFER;

	if (FAILED(hr))
	{
		*pszBuffer = TEXT('\0');
		return NULL;
	}
	
	return pszBuffer;
}



HRESULT ConfigIniSection::CreateConfig(REFGUID configId, LPCTSTR pszFilePath, LPCTSTR pszSection, const ConfigIniSection::CONFIGITEM *pItemList, INT nItemsCount, ConfigIniSection **ppConfig)
{
	if (NULL == ppConfig) 
		return E_POINTER;
	*ppConfig = NULL;

	if (NULL == pszSection || TEXT('\0') == *pszSection || 
		NULL == pItemList || nItemsCount < 1)
		return E_INVALIDARG;

	*ppConfig = new ConfigIniSection(configId, pszFilePath, pszSection, pItemList, nItemsCount);
	return S_OK;
}

ConfigIniSection::ConfigIniSection(REFGUID configId, LPCTSTR pszFilePath, LPCTSTR pszSection, const ConfigIniSection::CONFIGITEM *pItemList, INT nItemsCount) : 
	ref(1), path(NULL), section(NULL), guid(configId), items(pItemList), itemsCount(nItemsCount)
{
	path = (IS_INTRESOURCE(pszFilePath)) ? (LPTSTR)pszFilePath : StrDup(pszFilePath);
	section = StrDup(pszSection);
}

ConfigIniSection::~ConfigIniSection()
{
	if (NULL != path && !IS_INTRESOURCE(path))
		LocalFree(path);
	
	if (NULL != section)
		LocalFree(section);
}

STDMETHODIMP_(ULONG) ConfigIniSection::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ConfigIniSection::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

ConfigIniSection *ConfigIniSection::MakeProfileInstance(Profile *profile)
{
	TCHAR szBuffer[MAX_PATH * 2];
	LPCTSTR pszPath = path;

	if (IS_INTRESOURCE(path))
	{
		switch((DWORD)(DWORD_PTR)path)
		{
			case FILEPATH_WINAMPINI:		
				Plugin_GetWinampIniFile(szBuffer, ARRAYSIZE(szBuffer)); 
				break;
			case FILEPATH_PLUGININI:	
				GetPluginIniPath(szBuffer, ARRAYSIZE(szBuffer)); 
				break;
			case FILEPATH_MEDIALIBINI:	
				GetMediaLibraryIniPath(szBuffer, ARRAYSIZE(szBuffer)); 
				break;
			case FILEPATH_PROFILEINI:
				profile->GetFilePath(szBuffer, ARRAYSIZE(szBuffer)); 
				break;
			default:
				szBuffer[0] = TEXT('\0');
				break;
		}
		pszPath = szBuffer;
	}
	
	if (TEXT('\0') == *pszPath)
		return NULL;
	
	return new ConfigIniSection(guid, pszPath, section, items, itemsCount);
}


STDMETHODIMP ConfigIniSection::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IConfiguration))
		*ppvObject = (IConfiguration*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}


STDMETHODIMP ConfigIniSection::CreateInstance(REFIID configUid, Profile *profile, IConfiguration **ppConfig)
{
	if (IsEqualIID(configUid, guid))
	{
		if (IS_INTRESOURCE(path))
			*ppConfig = MakeProfileInstance(profile);
		else
			*ppConfig = this;
		
		if (NULL == *ppConfig)
			return E_OUTOFMEMORY;
		
		return S_OK;
	}
	return E_NOINTERFACE; // return if we have no idea aobut this interface
}

STDMETHODIMP ConfigIniSection::ResolveKeyString(LPCSTR pszKey, LPCSTR *ppszKnownKey)
{
	if (NULL == ppszKnownKey)
		return E_POINTER;

	if (NULL == pszKey || IS_INTRESOURCE(pszKey))
		return E_INVALIDARG;

	*ppszKnownKey = MAKEINTRESOURCEA(0);
	for (int i = 0; i < itemsCount; i++)
	{
		KEYLOOKUP(items[i].configKey, pszKey, *ppszKnownKey);
	}
	return E_NOTIMPL;
}


STDMETHODIMP ConfigIniSection::ReadString(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax)
{
	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			return (CONFIGITEM_TYPE_STRING == items[i].valueType) ? 
					ReadIniString(items[i].configKeyString, items[i].pszDefaultValue, pszBufferOut, cchBufferMax) : E_UNEXPECTED;
		}
	}
	return E_NOTIMPL;
}
STDMETHODIMP ConfigIniSection::ReadInt(LPCSTR pszKey, INT *pnValue)
{
	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			return (CONFIGITEM_TYPE_INT == items[i].valueType) ? 
					ReadIniInteger(items[i].configKeyString, items[i].nDefaultValue, pnValue) : E_UNEXPECTED;
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP ConfigIniSection::WriteString(LPCSTR pszKey, LPCTSTR pszBuffer)
{	
	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			return (CONFIGITEM_TYPE_STRING == items[i].valueType) ? 
					WriteIniString(items[i].configKeyString, pszBuffer) : E_UNEXPECTED;
		}
	}
	return E_NOTIMPL;
}

STDMETHODIMP ConfigIniSection::WriteInt(LPCSTR pszKey, INT nValue)
{
	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			return (CONFIGITEM_TYPE_INT == items[i].valueType) ? 
					WriteIniInteger(items[i].configKeyString, nValue) : E_UNEXPECTED;
		}
	}
	return E_NOTIMPL;
}


STDMETHODIMP ConfigIniSection::GetDefaultString(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax)
{
	*pszBufferOut = TEXT('\0');

	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			if (CONFIGITEM_TYPE_STRING != items[i].valueType)
				return E_UNEXPECTED;
	
			if (IS_INTRESOURCE(items[i].pszDefaultValue))
			{
				WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)items[i].pszDefaultValue, pszBufferOut, cchBufferMax);
				return S_OK;
			}

			return StringCchCopy(pszBufferOut, cchBufferMax, items[i].pszDefaultValue);
		}
	}
	return E_NOTIMPL;
}
STDMETHODIMP ConfigIniSection::GetDefaultInt(LPCSTR pszKey, INT *pnValue)
{
	if (NULL == pnValue)
		return E_INVALIDARG;
	
	if (!IS_INTRESOURCE(pszKey))
	{
		HRESULT hr = ResolveKeyString(pszKey, &pszKey);
		if (FAILED(hr)) return hr;
	}

	for (int i = 0; i < itemsCount; i++)
	{
		if (items[i].configKey == pszKey) 
		{
			if (CONFIGITEM_TYPE_INT == items[i].valueType) 
			{				
				*pnValue = items[i].nDefaultValue;
				return S_OK;
			}
			return E_UNEXPECTED;
		}
	}
	return E_NOTIMPL;
}


HRESULT ConfigIniSection::ReadIniInteger(LPCTSTR pszIniKey, int nDefault, int *pnValue)
{
	if (NULL == pnValue) return E_INVALIDARG;
	*pnValue = GetPrivateProfileInt(section, pszIniKey, nDefault, path);
	return S_OK;
}

HRESULT ConfigIniSection::ReadIniString(LPCTSTR pszIniKey, LPCTSTR pszDefault, LPTSTR pszBufferOut, INT cchBufferMax)
{
	LPCTSTR pszTestLame = TEXT("__no_string_test__");
	if (NULL == pszBufferOut || cchBufferMax < 1)
		return E_INVALIDARG;

	pszBufferOut[0] = TEXT('\0');
	INT len = GetPrivateProfileString(section, pszIniKey, pszTestLame, pszBufferOut, cchBufferMax, path);
	if (len == (cchBufferMax - 1))
	{
		pszBufferOut[0] = TEXT('\0');
		return STRSAFE_E_INSUFFICIENT_BUFFER;
	}
	pszBufferOut[len + 1] = TEXT('\0');
	if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, pszTestLame, -1, pszBufferOut, -1))
	{
		TCHAR szDefault[4096];
		if (IS_INTRESOURCE(pszDefault))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszDefault, szDefault, ARRAYSIZE(szDefault));
			pszDefault = szDefault;
		}
		
		if (NULL != pszDefault) 
			return StringCchCopyEx(pszBufferOut, cchBufferMax, pszDefault, NULL, NULL, STRSAFE_IGNORE_NULLS);
		
		pszBufferOut[0] = TEXT('\0');
		return S_OK;
	}
	return S_OK;
}

HRESULT ConfigIniSection::WriteIniString(LPCTSTR pszIniKey, LPCTSTR pszBuffer)
{	
	return (WritePrivateProfileString(section, pszIniKey, pszBuffer, path)) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

HRESULT ConfigIniSection::WriteIniInteger(LPCTSTR pszIniKey, INT nValue)
{
	TCHAR buf[32];
	StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("%d"), nValue);
	return WriteIniString(pszIniKey, buf);
}

STDMETHODIMP ConfigIniSection::Flush(void)
{
	return S_OK;
}