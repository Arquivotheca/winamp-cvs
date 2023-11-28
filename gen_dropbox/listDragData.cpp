#include "./listDragData.h"

static ListViewDragDropData *pActiveDragData = NULL;

ListViewDragDropData::ListViewDragDropData(HWND activeHwnd, IClipboardFormatProcessor *activeProcessor) :
		hwnd(activeHwnd), scroller(activeHwnd), insertMark(activeHwnd), processor(activeProcessor)
{
	if (NULL != processor)
		processor->AddRef();
}

ListViewDragDropData::~ListViewDragDropData()
{
	if (NULL != processor) 
		processor->Release();
}


DWORD ListViewDragDropData::Process(POINTL *pptl, DWORD dwAllowedEffect)
{
	if (NULL == processor)
		return DROPEFFECT_NONE;
	
	DWORD effect;

	if (0 != (DROPEFFECT_COPY & dwAllowedEffect))
		effect = DROPEFFECT_COPY;
	else if (0 != (DROPEFFECT_LINK & dwAllowedEffect))
		effect = DROPEFFECT_LINK;
	else
		effect = DROPEFFECT_NONE;

	POINT pt = { pptl->x, pptl->y };
	
	if (scroller.Scroll(pt))
	{
		insertMark.Remove(TRUE);
		
		effect |= DROPEFFECT_SCROLL;
	}
	else 
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		insertMark.Display(pt, 100);
	}
	return effect;
}
void ListViewDragDropData::RemoveMark(BOOL bRedraw)
{
	insertMark.Remove(bRedraw);
}


IClipboardFormatProcessor *ListViewDragDropData::GetProcessor() 
{ 
	if (NULL != processor)
		processor->AddRef();
	return processor;
}

INT ListViewDragDropData::GetInsertPoint(POINTL *pptl)
{
	scroller.Reset();
	POINT pt = { pptl->x, pptl->y };
	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
	insertMark.Display(pt, 0);
	return insertMark.GetPosition();
}

BOOL ListViewDragDropData::QueryIMarkActive()
{
	return insertMark.IsDisplayed();
}