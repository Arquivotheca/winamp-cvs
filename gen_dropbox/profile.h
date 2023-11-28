#ifndef NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILE_HEADER
#define NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IConfiguration;
class FilterPolicy;

typedef enum __CONFIGITEMTEMPLATETYPE
{
	ConfigItemTypeSection = 0,
	ConfigItemTypeString = 1,
	ConfigItemTypeInteger = 2,
	ConfigItemTypeFilterPolicy = 3,
} CONFIGITEMTEMPLATETYPE;

typedef struct __CONFIGITEMTEMPLATE
{
	LPCSTR	valueName;
	INT		valueType;
	union 
	{	
		LPCTSTR stringValue; 
		INT integerValue; 
		const GUID *guidValue;};
} CONFIGITEMTEMPLATE;


typedef struct __PROFILETEMPLATE
{
	UUID	profileUid;
	LPCTSTR	name;
	LPCTSTR	description;
	INT		itemsCount;
	const CONFIGITEMTEMPLATE *configItems;
} PROFILETEMPLATE;


class Profile
{
protected:
	Profile(const UUID &profileUid);
	~Profile();

public:
	static Profile *Create(HWND hwnd);
	static Profile *CreateEx(LPCTSTR pszName, LPCTSTR pszDescription);
	static Profile *CreateTemplate(const PROFILETEMPLATE *profileTemplate);
	static Profile *CreateCopy(Profile *source);
	static Profile *Load(LPCTSTR pszFilePath);

	static LPCTSTR GetFilePrefix();
	static HRESULT FormatPath(const UUID *profileUid, LPCTSTR pszBasePath, LPTSTR pszBuffer, INT cchBufferMax);
	static HRESULT MakeUniqueName(LPTSTR pszName, INT cchNameMax);

public:
	ULONG AddRef(void);
	ULONG Release(void);

	HRESULT GetUID(UUID *puid);
	HRESULT GetName(LPTSTR pszBuffer, INT cchBufferMax);
	HRESULT GetDescription(LPTSTR pszBuffer, INT cchBufferMax);
	HRESULT GetFilePath(LPTSTR pszBuffer, INT cchBufferMax);

	HRESULT SetName(LPCTSTR  pszName); // you can pass resourceId
	HRESULT SetDescription(LPCTSTR  pszDescription); // you can pass resourceId

	HRESULT Save();
	HRESULT Delete();

	HRESULT QueryConfiguration(REFGUID configId, IConfiguration **ppConfig);
	HRESULT  GetFilterPolicy(FilterPolicy **ppPolicy, BOOL forceReload);
	void Notify(UINT eventId);

protected:
	HRESULT CreateFileName();
	HRESULT WriteUID();

protected:
	ULONG ref;
	UUID uid;
	LPTSTR filePath;
	LPTSTR name;
	LPTSTR description;
	FilterPolicy *filterPolicy;
};


#endif //NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILE_HEADER
