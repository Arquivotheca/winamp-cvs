#ifndef NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/ptrlist.h"

class GLRoot;
class GLItem;
class GLGroup;
class GLCallback;
class GLStyle;

class __declspec(novtable) GLCallback
{
public:
	virtual void ItemStyleChanged(GLItem *item, UINT styleOld, UINT styleNew) = 0;
};

class __declspec(novtable) GLStyle
{
public:
	
	typedef enum
	{
		uiColorWindow=0,
		uiColorItem,
		uiColorItemText,
		uiColorItemHighlighted,
		uiColorItemTextHighlighted,
		uiColorItemDisabled,
		uiColorItemTextDisabled,
		uiColorItemPressed,
		uiColorItemTextPressed,
		uiColorGroup,
		uiColorGroupRight,
		uiColorGroupLine,
		uiColorGroupText,
		uiColorGroupTextShadow,
		uiColorGroupIcon,
		uiColorLast
	} uiColors;

	typedef enum
	{
		uiFontItem = 0,
		uiFontGroup,
		uiFontTitle,
		uiFontLast
	} uiFonts;

	typedef enum
	{
		uiMetricLevelOffset = 0,
		uiMetricRadiobuttonCX,
		uiMetricRadiobuttonCY,
		uiMetricItemFontCY,
		uiMetricTitleFontCY,
		uiMetricGroupFontCY,
		uiMetricIconCX,
		uiMetricIconCY,
		uiMetricItemInterval,
		uiMetricGroupInterval,
		uiMetricLast,
	} uiMetrics;

	typedef enum
	{
		updateFlagThemes		= 0x0001,
		updateFlagFonts		= 0x0002,
		updateFlagColors	= 0x0004,
		updateFlagImages		= 0x0008,
		updateFlagMetrics	= 0x0010,
		updateFlagAll		= (updateFlagThemes | updateFlagFonts | updateFlagColors | updateFlagImages),
	} updateFlags;

public:
	virtual COLORREF GetColor(UINT colorIndex) = 0;
	virtual HFONT GetFont(UINT fontIndex) = 0;
	virtual INT GetMetrics(UINT metricIndex) = 0;
	virtual BOOL DrawThemeBackground(HDC hdc, INT iPartId, INT iStateId, const RECT *pRect, const RECT *pClipRect) = 0;
	virtual BOOL DrawIcon(HDC hdc, INT iconId, const RECT *prc) = 0;
	virtual void Update(HWND hwndHost, UINT updateFlags) = 0;
};

class __declspec(novtable) GLView
{
public:
	virtual void Invalidate(GLItem *item) = 0;
	virtual GLStyle *GetStyle(void) = 0;
	virtual HWND GetHost(void) = 0;
};

class __declspec(novtable) GLItem
{
public:
	typedef enum
	{	
		// states
		FlagStateMask		= 0x000000FF,
		FlagStateDisabled	= 0x00000001, 
		FlagStateHot		= 0x00000002,
		FlagStatePressed	= 0x00000004,

		FlagStateButtonMask	= 0x000000F0,
		FlagStateLButton	= 0x00000010,
		FlagStateRButton	= 0x00000020,
		
		
		// types
		FlagTypeMask			= 0x0000FF00,
		
		// type specific states/values
		FlagValueMask		= 0xFFFF0000,

	} Flags;

	typedef enum 
	{
		DrawFlagNormal = 0x00000000,
		DrawFlagFocus = 0x00000001,
		DrawFlagDisabled = 0x00000002,
	} DrawFlags;

protected:
	GLItem(INT itemId, LPCTSTR pszTitle, UINT itemFlags);
	virtual ~GLItem();

public:
	ULONG AddRef();
	ULONG Release();
	INT GetId() { return id; }
	UINT GetFlags() { return flags; }
	void SetFlags(UINT newFlags, UINT flagsMask);
	GLRoot *GetRoot();
	INT GetLevel();
	UINT GetType();
	UINT GetValue();
	UINT GetState();
	GLItem *Next();
	GLItem *Previous();
	GLItem *GetParent() { return parent; }
	HRESULT SetTitle(LPCTSTR pszTitle);
	
	virtual BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags) = 0;
	virtual INT GetHeight(GLStyle *style) = 0;
	virtual BOOL AdjustRect(GLStyle *style, RECT *prcItem) = 0;
	
	virtual void MouseEnter(GLView *view) = 0;
	virtual void MouseLeave(GLView *view) = 0;
	virtual void MouseMove(GLView *view, const RECT *prcItem, POINT pt) = 0;
	virtual BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt) = 0; // return FALSE to prevent click
	virtual void LButtonUp(GLView *view, const RECT *prcItem, POINT pt) = 0;
	virtual void LButtonClick(GLView *view) = 0;

	virtual void KeyDown(GLView *view, UINT vKey) = 0;
	virtual void KeyUp(GLView *view, UINT vKey) = 0;
	virtual void KeyPressed(GLView *view, UINT vKey) = 0;

	virtual void StyleChanged(GLView *view, UINT updateFlags) = 0;
	
private:
	friend class GLGroup;
	friend static int __cdecl GLGroup_SortComparer(const void *elem1, const void *elem2);

protected:
	LONG	ref;
	INT		id;
	UINT	flags;
	LPTSTR	pszText;
	GLItem *parent;
	
};

class GLGroup : public GLItem
{
public:
	typedef enum
	{		
		// group types
		FlagTypeGroup	= 0x00000100,
		// group values
		FlagGroupExpanded	= 0x00000000,
		FlagGroupCollapsed	= 0x00010000,
	} Flags;

	typedef BOOL (CALLBACK *ChildEnumProc)(GLItem* /*item*/, ULONG_PTR /*user*/);

protected:
	GLGroup(INT groupId, INT iconId, LPCTSTR pszTitle, UINT groupFlags);
	~GLGroup();
public:
	static GLGroup *CreateInstance(INT groupId, INT iconId, LPCTSTR pszTitle, UINT groupFlags);

public:
	BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags);
	INT GetHeight(GLStyle *style);
	BOOL AdjustRect(GLStyle *style, RECT *prcItem);

	void MouseEnter(GLView *view);
	void MouseLeave(GLView *view);
	void MouseMove(GLView *view, const RECT *prcItem, POINT pt);
	BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonUp(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonClick(GLView *view);

	void KeyDown(GLView *view, UINT vKey);
	void KeyUp(GLView *view, UINT vKey);
	void KeyPressed(GLView *view, UINT vKey);

	void StyleChanged(GLView *view, UINT updateFlags);

	GLItem *GetChild(size_t index);
	size_t GetDirectChildCount();
	size_t GetVisibleChildCount();
	BOOL EnumerateChidlren(ChildEnumProc proc, ULONG_PTR user);

	GLItem *NextChild(GLItem *item);
	GLItem *PreviousChild(GLItem *item);

	GLItem *First();
	GLItem *Last();
	void Sort();

	size_t InsertItem(size_t index, GLItem *item);

protected:
//	friend class GLRoot;
	
	BOOL LookupItem(size_t flatIndex, size_t *parentIndex, GLItem **itemOut);
	BOOL LookupIndex(const GLItem *item, size_t *index);
	
protected:
	typedef nu::PtrList<GLItem> LGList;
	LGList children;
	INT iconId;
};


class GLRoot : public GLGroup
{
protected:
	GLRoot();
	~GLRoot();

public:
	static GLRoot *Create();

	BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags);
	INT GetHeight(GLStyle *style);
	BOOL AdjustRect(GLStyle *style, RECT *prcItem);

	void MouseEnter(GLView *view);
	void MouseLeave(GLView *view);
	void MouseMove(GLView *view, const RECT *prcItem, POINT pt);
	BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonUp(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonClick(GLView *view);

	void KeyDown(GLView *view, UINT vKey);
	void KeyUp(GLView *view, UINT vKey);
	void KeyPressed(GLView *view, UINT vKey);

	void StyleChanged(GLView *view, UINT updateFlags);

	void RegisterCallback(GLCallback *callback);
	GLItem *FindItem(INT flatIndex);
	INT FindIndex(const GLItem *item);

	void NotifyStyleChanged(GLItem *item, UINT styleOld, UINT styleNew);


protected:
	GLCallback *callback;
};


class GLButton : public GLItem
{
public:
	typedef enum
	{
		// button types
		FlagTypePushbutton	= 0x00000200,
		FlagTypeRadiobutton	= 0x00000300,
		FlagTypeCheckbox	= 0x00000400,
		
		// button values 
		FlagButtonUnchecked	= 0x00000000,
		FlagButtonChecked	= 0x00010000,
		FlagButtonMixed		= 0x00020000,
	} Flags;

protected:
	typedef enum
	{
		radioButtonMarginLeft = 2,
		radioButtonMarginTop = 2,
		radioButtonMarginRight = 2,
		radioButtonMarginBottom = 2,
		radioButtonTextOffset = 4,
		radioButtonHotTail = 8,
	} RadiobuttonMetrics;

	GLButton(INT itemId, LPCTSTR itemTitle, UINT itemFlags);
	~GLButton();

public:
	static GLButton *CreateRadiobutton(INT itemId, LPCTSTR itemTitle, BOOL checked);


public:
	BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags);
	INT GetHeight(GLStyle *style);
	BOOL AdjustRect(GLStyle *style, RECT *prcItem);

	void MouseEnter(GLView *view);
	void MouseLeave(GLView *view);
	void MouseMove(GLView *view, const RECT *prcItem, POINT pt);
	BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonUp(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonClick(GLView *view);

	void KeyDown(GLView *view, UINT vKey);
	void KeyUp(GLView *view, UINT vKey);
	void KeyPressed(GLView *view, UINT vKey);

	void StyleChanged(GLView *view, UINT updateFlags);

	void MarkRadioChecked(GLView *view);
	
protected:
	BOOL GetRadioBtnRect(GLStyle *style, const RECT *itemRect, RECT *buttonRect);
	void OnEnter(GLView *view);
	void OnLeave(GLView *view);
	void OnButtonDown(GLView *view, UINT mouseButton);
	void OnButtonUp(GLView *view, UINT mouseButton);
	
protected:
	INT textWidth;
};

class GLButtonEx : public GLButton
{

protected:
	GLButtonEx(INT itemId, LPCTSTR itemTitle, LPCTSTR itemDescription, UINT itemFlags);
	~GLButtonEx();

public:
	static GLButtonEx *CreateRadiobutton(INT itemId, LPCTSTR itemTitle, LPCTSTR itemDescription, BOOL checked);

public:
	BOOL Draw(GLStyle *style, HDC hdc, const RECT *prcItem, UINT drawFlags);
	INT GetHeight(GLStyle *style);
	BOOL AdjustRect(GLStyle *style, RECT *prcItem);

	void MouseEnter(GLView *view);
	void MouseLeave(GLView *view);
	void MouseMove(GLView *view, const RECT *prcItem, POINT pt);
	BOOL LButtonDown(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonUp(GLView *view, const RECT *prcItem, POINT pt);
	void LButtonClick(GLView *view);

protected:
	LPTSTR description;
};


#define NWC_GRPLSTVIEW		TEXT("NullsoftGroupedListView_01")

BOOL GroupedListView_RegisterClass(HINSTANCE hInstance);
HWND GroupedListView_CreateWindow(DWORD dwExStyle, DWORD dwStyle, INT x, INT y, INT cx, INT cy, 
							HWND hwndParent, INT controlId, HINSTANCE hInstance, GLRoot *root, LPCTSTR pszPngImage);

#define GLVM_FIRST			(WM_USER)
#define GLVM_SETROOT			(GLVM_FIRST + 0) // lParam = (LPARAM)(GLRoot*)groupedListRoot;
#define GLVM_SETBITMAP		(GLVM_FIRST + 1) // wParam = (WPARAM)(HINSTANCE)resourceInstance, lParam = (LPARAM)(LPCTSTR)resourceName;

#define GLVN_FIRST			(0)

typedef struct __NMGLITEM
{
	NMHDR hdr;
	GLItem	*item;
	UINT styleOld;
	UINT styleNew;
} NMGLITEM;

#define GLVN_ITEMSTYLECHANGED		(GLVN_FIRST + 0)  // NMGLITEM

typedef struct __NMGLCOLOR
{
	NMHDR		hdr;
	COLORREF	rgb;
	UINT		colorId;
} NMGLCOLOR;

#define GLVN_COLORCHANGING		(GLVN_FIRST + 1)  // NMGLCOLOR


#endif NULLOSFT_DROPBOX_PLUGIN_GROUPED_LIST_HEADER