#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumninfo.h"


#define COLUMN_WIDTH_MIN_DLU		(-(8*4))

class ColumnInfo : public ifc_viewcolumninfo
{
public:
	typedef HRESULT (*CreateColumnCallback)(ifc_viewcolumninfo *columnInfo, 
											ifc_dataprovider *provider, 
											ifc_viewcolumn **instance);
	

protected:
	ColumnInfo(const char *_name, CreateColumnCallback _callback);
	~ColumnInfo();

public:
	
	static HRESULT CreateInstance(const char *name, 
						  	      const wchar_t *displayName,
								  long width,
								  long widthMin,
								  long widthMax,
								  AlignMode alignMode,
								  const char *sortRule,
								  CreateColumnCallback createColumnCallback,
								  ColumnInfo **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcolumninfo */
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetWidth(long *width);
	HRESULT GetMinWidth(long *width);
	HRESULT GetMaxWidth(long *width);
	AlignMode GetAlignMode();
	const char *GetSortRule();
	HRESULT CreateColumn(ifc_dataprovider *provider, ifc_viewcolumn **instance);

public:
	HRESULT SetDisplayName(const wchar_t *displayName);
	HRESULT SetSortRule(const char *sortRule);

protected:
	size_t ref;
	char *name;
	wchar_t *displayName;
	long width;
	long widthMin;
	long widthMax;
	AlignMode alignMode;
	char *sortRule;
	CreateColumnCallback createColumnCallback;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_HEADER