#ifndef _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_HEADER_WINDOW_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_HEADER_WINDOW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif


#include "./listView.h"

class ListViewHeader : public ViewWindow
{
protected:
	ListViewHeader(HWND hwnd, ListView *listView, ifc_viewcontents *contents);
	~ListViewHeader();

public:
	static HRESULT CreateInstance(ListView *listView,
								  ListViewHeader **instance);

protected:
	HRESULT SwapColumns(int iOrder1, int iOrder2);

protected:
	// ViewWindow
	void OnContextMenu(HWND targetWindow, long cursor);
	HRESULT ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result);

	// Reflected Messages
	LRESULT OnNotification(NMHDR *notification);

	// Notifications
	virtual BOOL OnItemChanging(int iItem, int iButton, HDITEM *item);
	virtual void OnItemChanged(int iItem, int iButton, HDITEM *item);
	virtual BOOL OnBeginTrack(int iItem, int iButton, HDITEM *item);
	virtual void OnEndTrack(int iItem, int iButton, HDITEM *item);
	virtual BOOL OnBeginDrag(int iItem, int iButton, HDITEM *item);
	virtual BOOL OnEndDrag(int iItem, int iButton, HDITEM *item);



protected:
	ListView *listView;
	LengthConverter lengthConverter;
	

};

#endif // _NULLSOFT_WINAMP_DATAVIEW_LIST_VIEW_HEADER_WINDOW_HEADER