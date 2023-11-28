#ifndef NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGDROPDATA_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGDROPDATA_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include "./listInsertMark.h"
#include "./listDragScroll.h"
#include "./cfpInterface.h"


class ListViewDragDropData
{
public:
	ListViewDragDropData(HWND activeHwnd, IClipboardFormatProcessor *activeProcessor);
	~ListViewDragDropData();

public:
	DWORD Process(POINTL *pptl, DWORD dwAllowedEffect);
	IClipboardFormatProcessor *GetProcessor();
	INT GetInsertPoint(POINTL *pptl);
	BOOL QueryIMarkActive();
	void RemoveMark(BOOL bRedraw);

private:
	HWND hwnd;
	IClipboardFormatProcessor *processor;
	ListViewDragScroll scroller;
	ListViewInsertMark insertMark;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_LISTVIEW_DRAGDROPDATA_HEADER