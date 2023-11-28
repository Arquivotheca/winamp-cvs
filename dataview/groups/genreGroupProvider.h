#ifndef _NULLSOFT_WINAMP_DATAVIEW_GENRE_GROUP_PROVIDER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GENRE_GROUP_PROVIDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../ifc_groupprovider.h"


class GenreGroupProvider : public ifc_groupprovider
{

protected:
	GenreGroupProvider();
	~GenreGroupProvider();

public:
	static HRESULT CreateInstance(ifc_groupprovider **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_groupprovider */
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT Bind(ifc_dataprovider *provider);
	HRESULT ResolveNames(const char **names, size_t count, size_t *valueIds);
	HRESULT CreateGroup(LCID localeId, ifc_dataobject *object, ifc_dataobject **group);
	int CompareObjects(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2);
	HRESULT GetCounterText(wchar_t *buffer, size_t bufferSize);
	HRESULT GetEmptyText(wchar_t *buffer, size_t bufferSize); 
	HRESULT CreateSummaryGroup(ifc_dataobject **group);

public:
	size_t ResolveName(const char *name);

protected:
	size_t ref;
	size_t artistId;
	size_t artistKeyId;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_GENRE_GROUP_PROVIDER_HEADER