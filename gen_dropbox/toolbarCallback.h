#ifndef NULLOSFT_DROPBOX_PLUGIN_TOOLBARCALLBACK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_TOOLBARCALLBACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class Toolbar;

class __declspec(novtable) ToolbarCallback
{
public:
	typedef enum
	{
		NotSupported = 0,
		Success = 1,
		Failed = -1,
	} CallbackResult;

protected:
	ToolbarCallback(){}
	virtual ~ToolbarCallback(){}

public:
	virtual void OnDestroy(Toolbar *instance) = 0;
	virtual INT TrackMouseLeave(Toolbar *instance) = 0;
	virtual void CancelTrackMouseLeave(Toolbar *instance) = 0;
	virtual void Invalidate(const RECT *prcInvalid) = 0;
	virtual void ShowTip(LPCTSTR pszText, const RECT *prcBounds) = 0;
	virtual void OnCommand(Toolbar *instance, UINT commandId) = 0;
	virtual HMENU ResolveMenuId(INT menuId) = 0;
	virtual void ReleaseMenu(INT menuId, HMENU hMenu) = 0;
	virtual INT TrackPopupMenuEx(HMENU hMenu, UINT menuFlags, INT x, INT y, LPTPMPARAMS lptpm) = 0;
};


#endif //NULLOSFT_DROPBOX_PLUGIN_TOOLBARCALLBACK_HEADER