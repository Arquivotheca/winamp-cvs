#ifndef _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {FA35A17F-023F-480f-9913-CEAC77E0094B}
static const GUID IFC_ViewToolbarItem = 
{ 0xfa35a17f, 0x23f, 0x480f, { 0x99, 0x13, 0xce, 0xac, 0x77, 0xe0, 0x9, 0x4b } };

#include <bfc/dispatch.h>
#include "./ifc_image.h"
#include "./dataValue.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewtoolbaritem : public Dispatchable
{
public:
	typedef enum AppearanceFlags
	{
		AppearanceFlag_Normal = 0,
		AppearanceFlag_OnlyChevron = (1 << 0),	// item always displayed in chevron
		AppearanceFlag_NoChevron = (1 << 1),	// item never appears in chevorn
		AppearanceFlag_Permanent = (1 << 2),	// even if item hidden - space is occupied
	} AppearanceFlags;

protected:
	ifc_viewtoolbaritem() {}
	~ifc_viewtoolbaritem() {}

public:
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	AppearanceFlags GetAppearance();
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETDISPLAYNAME = 20,
		API_GETDESCRIPTION = 30,
		API_GETICON = 40,
		API_GETAPPEARANCE = 50,
	};
};

inline const char *ifc_viewtoolbaritem::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewtoolbaritem::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewtoolbaritem::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDESCRIPTION, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewtoolbaritem::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, buffer, bufferSize, width, height);
}

inline ifc_viewtoolbaritem::AppearanceFlags ifc_viewtoolbaritem::GetAppearance()
{
	return _call(API_GETAPPEARANCE, (AppearanceFlags)AppearanceFlag_Normal);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_TOOLBAR_ITEM_INTERFACE_HEADER