#include "./main.h"
#include "./detailsView.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./itemViewMeta.h"
#include "./itemViewManager.h"
#include "./imageLoader.h"


static INT_PTR CALLBACK DetailsViewEditor_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return 0;
}


class DetailsViewMeta: public DropboxViewMeta
{

public:
	INT GetId() { return DETAILSVIEW_ID; }
	LPCTSTR GetName() { return DETAILSVIEW_NAME; }
	
	HRESULT GetTitle(LPTSTR pszBuffer, INT cchBufferMax)
	{
		if (NULL == pszBuffer) return E_POINTER;
		WASABI_API_LNGSTRINGW_BUF(IDS_DETAILSVIEW, pszBuffer, cchBufferMax);
		return (TEXT('\0') != pszBuffer) ? S_OK : E_FAIL;
	}
	HRESULT GetDescirption(LPTSTR pszBuffer, INT cchBufferMax)
	{
		if (NULL == pszBuffer) return E_POINTER;
		WASABI_API_LNGSTRINGW_BUF(IDS_DETAILSVIEW_DESCRIPTION, pszBuffer, cchBufferMax);
		return (TEXT('\0') != pszBuffer) ? S_OK : E_FAIL;
	}
	HBITMAP LoadPreview()
	{
		return ImageLoader_LoadPng(plugin.hDllInstance, MAKEINTRESOURCE(IDR_DETAILSVIEW_PREVIEWIMAGE), NULL, NULL);
	}
	BOOL HasEditor()
	{
		return FALSE;
	}

	INT_PTR ShowEditor(HWND hParent, Profile *profile)
	{
		return 0;
	}

	HWND CreateView(UINT styleEx, UINT style, INT x, INT y, INT cx, INT cy, HWND hParent, INT controlId, HINSTANCE hInstance)
	{
		HWND hView;
		hView = CreateWindowEx(styleEx, WC_LISTVIEW, NULL,
					style | LVS_REPORT | LVS_OWNERDATA | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS, 
					x, y, cx, cy, hParent, (HMENU)(INT_PTR)controlId, hInstance, NULL);
		
		if (NULL != hView)
		{
			DetailsView *pView = new DetailsView(hView);
			if (!pView->IsAttached())
			{
				delete(pView);
				hView = NULL;
			}
		}
		return hView;
	}

};

static DetailsViewMeta detailsViewMetaInstance;
DropboxViewMeta *detailsViewMeta = &detailsViewMetaInstance;