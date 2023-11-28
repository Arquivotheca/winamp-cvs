#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_META_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_META_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class __declspec(novtable) DropboxViewMeta
{
public:
	virtual INT GetId() = 0;
	virtual LPCTSTR GetName() = 0;
	virtual HRESULT GetTitle(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual HRESULT GetDescirption(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual HBITMAP LoadPreview() = 0;
	virtual BOOL HasEditor() = 0;
	virtual INT_PTR ShowEditor(HWND hParent, Profile *profile) = 0;
	virtual HWND CreateView(UINT styleEx, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT controlId, HINSTANCE hInstance) = 0;
	

};

#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_META_HEADER