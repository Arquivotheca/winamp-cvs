#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C290351C-066A-4a9f-865A-FE3447D049C5}
static const GUID IFC_ViewColumnInfo = 
{ 0xc290351c, 0x66a, 0x4a9f, { 0x86, 0x5a, 0xfe, 0x34, 0x47, 0xd0, 0x49, 0xc5 } };

#include <bfc/dispatch.h>

class ifc_dataprovider;
class ifc_viewcolumn;


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcolumninfo : public Dispatchable
{
public:
	typedef enum AlignMode
	{		
		AlignMode_Default = -1,
		AlignMode_Left = 0,
		AlignMode_Right = 1,
		AlignMode_Center = 2,
	} AlignMode;
	
protected:
	ifc_viewcolumninfo() {}
	~ifc_viewcolumninfo() {}

public:
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize); 
	
	// if any width specified as negative it is in DLU
	HRESULT GetWidth(long *width);
	HRESULT GetMinWidth(long *width);
	HRESULT GetMaxWidth(long *width);
	
	AlignMode GetAlignMode();

	const char *GetSortRule();

	HRESULT CreateColumn(ifc_dataprovider *provider, ifc_viewcolumn **instance);
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETDISPLAYNAME = 20,
		API_GETWIDTH = 30,
		API_GETMINWIDTH = 40,
		API_GETMAXWIDTH = 50,
		API_GETALIGNMODE = 60,
		API_GETSORTRULE = 70,
		API_CREATECOLUMN = 80,
	};
};


inline const char * ifc_viewcolumninfo::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewcolumninfo::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer,bufferSize);
}

inline HRESULT ifc_viewcolumninfo::GetWidth(long *width)
{
	return _call(API_GETWIDTH, (HRESULT)E_NOTIMPL, width);
}

inline HRESULT ifc_viewcolumninfo::GetMinWidth(long *width)
{
	return _call(API_GETMINWIDTH, (HRESULT)E_NOTIMPL, width);
}

inline HRESULT ifc_viewcolumninfo::GetMaxWidth(long *width)
{
	return _call(API_GETMAXWIDTH, (HRESULT)E_NOTIMPL, width);
}

inline ifc_viewcolumninfo::AlignMode ifc_viewcolumninfo::GetAlignMode()
{
	return _call(API_GETALIGNMODE, (ifc_viewcolumninfo::AlignMode)AlignMode_Default);
}

inline const char *ifc_viewcolumninfo::GetSortRule()
{
	return _call(API_GETSORTRULE, (const char*)NULL);
}

inline HRESULT ifc_viewcolumninfo::CreateColumn(ifc_dataprovider *provider, ifc_viewcolumn **instance)
{
	return _call(API_CREATECOLUMN, (HRESULT)E_NOTIMPL, provider, instance);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_INTERFACE_HEADER