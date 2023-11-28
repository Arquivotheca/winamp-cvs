#ifndef NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_VIEW_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_VIEW_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/windowsTheme.h"


class GroupedListViewStyle: public GLStyle
{
public:
	GroupedListViewStyle();
	~GroupedListViewStyle();
public:
	COLORREF GetColor(UINT colorIndex)	{ return (colorIndex < ARRAYSIZE(szColors)) ? szColors[colorIndex] : RGB(255, 0, 255); }
	HFONT GetFont(UINT fontIndex)		{ return (fontIndex < ARRAYSIZE(szFonts)) ? szFonts[fontIndex] : NULL; }
	INT GetMetrics(UINT metricIndex)		{ return (metricIndex < ARRAYSIZE(szMetrics)) ? szMetrics[metricIndex] : 0; }

	BOOL DrawThemeBackground(HDC hdc, INT iPartId, INT iStateId, const RECT *pRect, const RECT *pClipRect);
	BOOL DrawIcon(HDC hdc, INT iconId, const RECT *prc);
	BOOL SetPngResource(HINSTANCE hInstance, LPCTSTR pszResource);

	void Update(HWND hwndHost, UINT updateFlags);
	
protected:
	void OpenThemes(HWND hwndHost);
	void CloseThemes();
	void LoadImages();
	void CloseImages();
	
	void LoadFonts(HWND hwndHost);
	void CloseFonts();

	void PopulateColors(HWND hwndHost);
	void PopulateMetrics(HDC hdc);

private:
	
	void PopulateBitmapMetrics();
	void PopulateThemeMetrics(HDC hdc);
	void PopulateFontMetrics(HDC hdc);

	

protected:
	COLORREF szColors[GLStyle::uiColorLast];
	HFONT szFonts[GLStyle::uiFontLast];
	INT szMetrics[GLStyle::uiMetricLast];
	
	HINSTANCE pngInstance;
	LPTSTR	pngResource;
	HBITMAP bitmap;

	void *buttonTheme;
		
};

class GroupedListView : public GLView, public GLCallback
{
public:
	GroupedListView(HWND hHost);
	~GroupedListView();

public:
	void Invalidate(GLItem *item);
	GLStyle *GetStyle(void) { return style; }
	HWND GetHost() { return hwnd; }
	void SetRoot(GLRoot *groupRoot);
	BOOL SetBitmap(HINSTANCE resourceInstance, LPCTSTR resourceName);
	void Paint(const PAINTSTRUCT *paintStruct);
	BOOL PaintItem(GLItem *item, const PAINTSTRUCT *paintStruct, RECT *paintRect, HRGN eraseRegion);
	void UpdateScrollInfo();
	void ScrollVert(UINT scrollCode, UINT trackPos);
	INT ScrollVertDelta(INT delta);
	GLItem *HitTest(POINT pt, RECT *prcItem);

	void MouseWheel(INT zDelta, UINT mouseFlags, POINT pt);
	void MouseMove(UINT mouseFlags, POINT pt);
	void MouseLeave();
	void LeftButtonDown(UINT mouseFlags, POINT pt);
	void LeftButtonUp(UINT mouseFlags, POINT pt);

	void KeyDown(UINT virtualKey, UINT keyFlags);
	void KeyUp(UINT virtualKey, UINT keyFlags);


	void UpdateUiState(UINT actionId, UINT stateId);

	void PageUp();
	void PageDown();

	void Select(GLItem *item);
	void SelectPrevious(GLItem *item, BOOL ensureVisible);
	void SelectNext(GLItem *item, BOOL ensureVisible);
	BOOL GetItemRect(GLItem *item, RECT *itemRect);
	void EnsureVisible(GLItem *item);

	BOOL UpdateStyle(UINT updateFlags);
	void FocusChanged(BOOL gainFocus);

	void SetFont(HFONT windowFont);
	HFONT GetFont();
	void *GetTheme();

	void ItemStyleChanged(GLItem *item, UINT styleOld, UINT styleNew);




private:
	LONG GetViewHeight(INT *minHeightOut, INT *maxHeightOut);
	void RemoveHighlight();
	void UpdateHighlight();


protected:
	HWND hwnd;
	HFONT hFont;
	GroupedListViewStyle *style;
	GLRoot *root;
	GLItem *highlighted;
	GLItem *pressed;
	GLItem *focused;
	INT lineHeight;
	INT	wheelCarryover;
	BOOL continuousScroll;

	HBRUSH backgroundBrush;
	void *listTheme;

	HBITMAP backBuffer;
	HDC		backDC;
};


#endif // NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_VIEW_HEADER
