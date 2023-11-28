#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_DETAILS_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_DETAILS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./baseListView.h"
#include "./dropWindowInternal.h"
#include <windows.h>
#include <commctrl.h>

#define DETAILSVIEW_ID		2
#define DETAILSVIEW_NAME		TEXT("detailsView")

class ConfigurationManager;

class DetailsView : BaseListView 
{
protected:
	DetailsView(HWND hView);
	virtual ~DetailsView();

public:
	STDMETHOD(ProcessNotification)(NMHDR*, LRESULT*);
	STDMETHOD_(DropboxViewMeta*, GetMeta)();
	STDMETHOD(ConfigChanged)(void);
	
	STDMETHOD(Save)(Profile *profile);
	STDMETHOD(Load)(Profile *profile);
	

protected:
	virtual LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(UINT uFlags, POINTS pts);

	void UpdateColumnsData();
	void UpdateListColumns();
	void OnGetDispInfo(NMLVDISPINFO *pdi);
	void OnColumnClick(NMLISTVIEW *pnmv); 
		
	
	
protected:
	friend class DetailsViewMeta;

protected:
	ACTIVECOLUMN columns[COLUMN_LAST + 1]; // we need this + 1 !!!
};

extern DropboxViewMeta *detailsViewMeta;




#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMVIEW_DETAILS_HEADER