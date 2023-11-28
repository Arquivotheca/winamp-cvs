#ifndef NULLOSFT_DROPBOX_PLUGIN_TOOLBARITEM_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_TOOLBARITEM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class Toolbar;

class __declspec(novtable) ToolbarItem
{
public:
	typedef enum
	{
		FlagDisabled = 0x0001,
        FlagPressed = 0x0002,
		FlagHighlighted = 0x0004,
		FlagChecked = 0x0010,
		FlagGrouped = 0x1000, // if this flag set item grouped with previos one (no spacing)
		FlagSpacer = 0x2000, // item treated as spacer (no click, force new line in menu) 
		FlagFlexSpacer = 0x4000, // item treated as spacer (no click, force new line in menu) 
	} ToolbarItemFlags;

protected:
	ToolbarItem(Toolbar *toolbar, INT itemWidth, UINT itemFlags) : width(itemWidth), flags(itemFlags)  {}
	virtual ~ToolbarItem() {}

public:
	INT GetWidth() { return width; }
	UINT GetFlags() { return flags; }
	void SetFlags(UINT newFlags, UINT flagsMask) 
	{ 
		flags = (flags & ~flagsMask) | (newFlags & flagsMask); 
	}

	virtual BOOL Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags) = 0;
	virtual INT GetTip(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual BOOL ButtonDown(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) = 0;
	virtual BOOL ButtonUp(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) = 0;

	virtual BOOL FillMenuInfo(Toolbar *toolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax) { return FALSE; }

protected:
	BOOL DrawIcon(Toolbar *toolbar, HDC hdc, INT imageId, const RECT *prc, UINT stateFlags);
protected:
	friend class Toolbar;

protected:
	INT width;
	UINT flags;
};

class ToolbarSpacer : public ToolbarItem
{
protected:
	ToolbarSpacer(Toolbar *toolbar, BOOL flexMode);
	~ToolbarSpacer() {}
public:
	virtual BOOL Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags) { return FALSE; }
	virtual INT GetTip(LPTSTR pszBuffer, INT cchBufferMax) { return 0; }
	virtual BOOL ButtonDown(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) { return FALSE; }
	virtual BOOL ButtonUp(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) { return FALSE; }

protected:
	friend class Toolbar;

};


class ToolbarSeparator : public ToolbarItem
{
protected:
	ToolbarSeparator();
	~ToolbarSeparator() {}
public:
	virtual BOOL Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags);
	virtual INT GetTip(LPTSTR pszBuffer, INT cchBufferMax) { return 0; }
	virtual BOOL ButtonDown(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) { return FALSE; }
	virtual BOOL ButtonUp(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags) { return FALSE; }
	BOOL FillMenuInfo(Toolbar *toolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);

protected:
	friend class Toolbar;

};

class ToolbarButton : public ToolbarItem
{
public:
	typedef enum
	{
		FlagDropdownButton = 0x00010000, // onClick button will request and display menu with id of commandId
	} ToolbarButtonFlags;

protected:
	ToolbarButton(Toolbar *toolbar, UINT commandId, INT imageId, LPCTSTR pszTitle, LPCTSTR pszDescription, UINT buttonFlags);
	virtual ~ToolbarButton();

public:
	BOOL Draw(Toolbar *toolbar, HDC hdc, const RECT *prcItem, const RECT *prcUpdate, UINT stateFlags);
	INT GetTip(LPTSTR pszBuffer, INT cchBufferMax);
	BOOL ButtonDown(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags);
	BOOL ButtonUp(Toolbar *toolbar, UINT mouseButton, POINT pt, UINT mouseFlags);

	BOOL FillMenuInfo(Toolbar *toolbar, MENUITEMINFO *pmii, LPWSTR pszBuffer, INT cchBufferMax);

protected:
	virtual void OnClick(Toolbar *toolbar);

protected:
	friend class Toolbar;
protected:
	LPTSTR	pszTitle;
	LPTSTR	pszDescription;
	INT		imageId;
	UINT	commandId;
};


class ToolbarChevron : public ToolbarButton
{
protected:
	ToolbarChevron();
	virtual ~ToolbarChevron() {}

protected:
	virtual void OnClick(Toolbar *toolbar);

protected:
	friend class Toolbar;
};
#endif // NULLOSFT_DROPBOX_PLUGIN_TOOLBARITEM_HEADER