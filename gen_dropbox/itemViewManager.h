#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_MANAGER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./itemViewMeta.h"

class DropboxViewManager
{
public:
	typedef BOOL (CALLBACK *EnumProc)(DropboxViewMeta* /*meta*/, ULONG_PTR /*param*/);

public:
	DropboxViewMeta *FindById(INT viewId);
	DropboxViewMeta *FindByName(LPCTSTR viewName);
	DropboxViewMeta *First();
	BOOL EnumerateViews(EnumProc proc, ULONG_PTR param);
};

extern DropboxViewManager *pluginViewManager;
#define PLUGIN_VIEWMNGR pluginViewManager


#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_MANAGER_HEADER