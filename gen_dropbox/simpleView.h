#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_SIMPLE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_SIMPLE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./baseListView.h"
#include "./dropWindowInternal.h"
#include "./itemPainter.h"
#include <windows.h>
#include <commctrl.h>

#define SIMPLEVIEW_ID		1
#define SIMPLEVIEW_NAME		TEXT("simpleView")

class SimpleView : public BaseListView 
{

protected:
	SimpleView(HWND hView);
	virtual ~SimpleView();


public:
	STDMETHOD(ProcessNotification)(NMHDR *pnmh, LRESULT *pResult);
	STDMETHOD(DrawItem)(DRAWITEMSTRUCT *pdis);
	STDMETHOD(MeasureItem)(MEASUREITEMSTRUCT *pmis);

	STDMETHOD_(DropboxViewMeta*, GetMeta)();
	STDMETHOD(ConfigChanged)(void);
	STDMETHOD(Save)(Profile *profile);
	STDMETHOD(Load)(Profile *profile);
	
protected:

	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void OnWindowPosChanged(WINDOWPOS *pwp);
	virtual LRESULT OnSetItemCount(INT cItems, DWORD dwFlags);

	void UpdateIndexColumnWidth();
	void OnLButtonDown(UINT uFlags, POINTS pts);
	void OnRButtonDown(UINT uFlags, POINTS pts);
	virtual void OnCommand(INT ctrlId, INT eventId, HWND hwndCtrl);
	STDMETHOD(SetSkinned)(BOOL bSkinned);
	void OnKeyDown(UINT vkCode, UINT flags);
	void OnSetFont(HFONT hFont, BOOL bRedraw);
	
	LRESULT OnListViewCustomDraw(NMLVCUSTOMDRAW *pcd);
	LRESULT OnPrePaint(NMLVCUSTOMDRAW *pcd);

	void PaintEmptyList(HDC hdc, RECT *prcPaint, BOOL fErase);

protected:
	friend class SimpleViewMeta;

protected:
	DWORD	flags;
	ItemPainter painter;
	LONG	prevWidth;
	LONG	itemHeight;
	BOOL	wpcReentryFilter;
};

extern DropboxViewMeta *simpleViewMeta;

EXTERN_C const GUID simpleViewSettingsGuid;

#define CFG_SHOWINDEX		MAKEINTRESOURCEA(1)
#define CFG_SHOWTYPEICON		MAKEINTRESOURCEA(2)
#define CFG_RCOLUMNSOURCE	MAKEINTRESOURCEA(3)

INT_PTR SimpleViewEditor_Show(HWND hParent, Profile *profile);


#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_SIMPLE_HEADER