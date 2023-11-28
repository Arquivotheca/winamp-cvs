#ifndef _NULLSOFT_WINAMP_DATAVIEW_STRING_SORT_KEY_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_STRING_SORT_KEY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_sortkey.h"
#include "./api_dataview.h"

class StringSortKey : public ifc_sortkey
{
protected:
	StringSortKey(LCID localeId, const void *key, size_t size);
	~StringSortKey();

public:
	static HRESULT CreateInstance(LCID localeId, 
								  const wchar_t *value, 
								  StringSortKeyFlags flags,
								  StringSortKey **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_datasortkey */
	const void *GetValue();
	size_t GetSize();
	LCID GetLocaleId();
	int Compare(const void *value, size_t size);
	
protected:
	size_t ref;
	LCID localeId;
	const void *value;
	size_t size;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_STRING_SORT_KEY_HEADER