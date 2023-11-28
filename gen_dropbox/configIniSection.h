#ifndef NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_INISECTION_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_INISECTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./configInterface.h"

#define FILEPATH_PROFILEINI		MAKEINTRESOURCE(0)
#define FILEPATH_WINAMPINI		MAKEINTRESOURCE(1)
#define FILEPATH_PLUGININI		MAKEINTRESOURCE(2)
#define FILEPATH_MEDIALIBINI		MAKEINTRESOURCE(3)


class ConfigIniSection : public IConfiguration
{
public:
	typedef enum __CONFIGITEM_VALUETYPE
	{
		CONFIGITEM_TYPE_STRING,
		CONFIGITEM_TYPE_INT,
	}CONFIGITEM_VALUETYPE;

	typedef struct __CONFIGITEM
	{
		LPCSTR configKey;
		LPCTSTR configKeyString;
		INT		valueType;
		union {	
			LPCTSTR pszDefaultValue; 
			INT nDefaultValue; };
	} CONFIGITEM;

public:
	static HRESULT CreateConfig(REFGUID configId, LPCTSTR pszFilePath, LPCTSTR pszSection, const ConfigIniSection::CONFIGITEM *pItemList, INT nItemsCount, ConfigIniSection **ppConfig);

protected:
	ConfigIniSection(REFGUID configId, LPCTSTR pszFilePath, LPCTSTR pszSection, const ConfigIniSection::CONFIGITEM *pItemList, INT nItemsCount);
	virtual ~ConfigIniSection();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IConfiguration ***/
	STDMETHOD(CreateInstance)(REFIID configUid, Profile *profile, IConfiguration **ppConfig);
	STDMETHOD(ResolveKeyString)(LPCSTR pszKey, LPCSTR *ppszKnownKey);
	STDMETHOD(ReadString)(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax);
	STDMETHOD(ReadInt)(LPCSTR pszKey, INT *pnValue);
	STDMETHOD(WriteString)(LPCSTR pszKey, LPCTSTR pszBuffer);
	STDMETHOD(WriteInt)(LPCSTR pszKey, INT nValue);
	STDMETHOD(GetDefaultString)(LPCSTR pszKey, LPTSTR pszBufferOut, INT cchBufferMax);
	STDMETHOD(GetDefaultInt)(LPCSTR pszKey, INT *pnValue);	
	STDMETHOD(Flush)(void);

protected:
	ConfigIniSection *MakeProfileInstance(Profile *profile);
	HRESULT ReadIniInteger(LPCTSTR pszIniKey, int nDefault, int *pnValue);
	HRESULT ReadIniString(LPCTSTR pszIniKey, LPCTSTR pszDefault, LPTSTR pszBufferOut, INT cchBufferMax);
	HRESULT WriteIniString(LPCTSTR pszIniKey, LPCTSTR pszBuffer);
	HRESULT WriteIniInteger(LPCTSTR pszIniKey, INT nValue);

protected:
	ULONG ref;
	GUID guid;
	LPTSTR path;
	LPTSTR section;
	const CONFIGITEM *items;
	INT	itemsCount;
	
};


#endif //NULLSOFT_DROPBOX_PLUGIN_CONIFGURATION_INISECTION_HEADER