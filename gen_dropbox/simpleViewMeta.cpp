#include "./main.h"
#include "./simpleView.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./itemViewMeta.h"
#include "./itemViewManager.h"
#include "./imageLoader.h"

class SimpleViewMeta: public DropboxViewMeta
{

public:
	INT GetId() { return SIMPLEVIEW_ID; }
	LPCTSTR GetName() { return SIMPLEVIEW_NAME; }
	
	HRESULT GetTitle(LPTSTR pszBuffer, INT cchBufferMax)
	{
		if (NULL == pszBuffer) return E_POINTER;
		WASABI_API_LNGSTRINGW_BUF(IDS_SIMPLEVIEW, pszBuffer, cchBufferMax);
		return (TEXT('\0') != pszBuffer) ? S_OK : E_FAIL;
	}
	HRESULT GetDescirption(LPTSTR pszBuffer, INT cchBufferMax)
	{
		if (NULL == pszBuffer) return E_POINTER;
		WASABI_API_LNGSTRINGW_BUF(IDS_SIMPLEVIEW_DESCRIPTION, pszBuffer, cchBufferMax);
		return (TEXT('\0') != pszBuffer) ? S_OK : E_FAIL;
	}
	HBITMAP LoadPreview()
	{
		return ImageLoader_LoadPng(plugin.hDllInstance, MAKEINTRESOURCE(IDR_SIMPLEVIEW_PREVIEWIMAGE), NULL, NULL);
	}
	BOOL HasEditor()
	{		
		return TRUE;
	}

	INT_PTR ShowEditor(HWND hParent, Profile *profile)
	{
		return SimpleViewEditor_Show(hParent, profile);
	}

	HWND CreateView(UINT styleEx, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT controlId, HINSTANCE hInstance)
	{
		HWND hView;
		hView = CreateWindowEx(styleEx, WC_LISTVIEW, NULL,
					style | LVS_REPORT | LVS_OWNERDATA | LVS_SHAREIMAGELISTS  | LVS_NOCOLUMNHEADER | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS, 
					x, y, cx, cy, hParent, (HMENU)(INT_PTR)controlId, hInstance, NULL);
		
		if (NULL != hView)
		{
			SimpleView *pView = new SimpleView(hView);
			if (!pView->IsAttached())
			{
				delete(pView);
				hView = NULL;
			}
		}
		return hView;
	}

};

static SimpleViewMeta simpleViewMetaInstance;
DropboxViewMeta *simpleViewMeta = &simpleViewMetaInstance;

