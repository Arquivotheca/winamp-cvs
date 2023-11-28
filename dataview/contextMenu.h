#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewmenuitem.h"
#include "./ifc_viewmenugroup.h"
#include "./ifc_viewactioncontextenum.h"
#include "./contextMenuEventHandler.h"

class ContextMenu 
{
protected:
	ContextMenu(ifc_viewactioncontextenum *contextEnum, ifc_viewmenugroup *root, BOOL extendedMode);
	~ContextMenu();

public:
	static HRESULT CreateInstance(ifc_viewactioncontextenum *contextEnum,
								  ifc_viewmenugroup *root,
								  BOOL extendedMode,
								  ContextMenu **instance);

	static HRESULT GetFromHandle(HMENU hMenu,
								 ContextMenu **instance);

public:
	size_t AddRef();
	size_t Release();
	
public:
	HRESULT GetRoot(ifc_viewmenugroup **group);
	HMENU GetHandle();
	HRESULT Show(unsigned int flags, int x, int y, HWND hwnd, TPMPARAMS *params);
	BOOL IsExtendedMode();

protected:
	HRESULT Init();
	HRESULT UpdateMenuInfo();
	HRESULT InsertItem(size_t positionHint, BOOL verifyPosition, ifc_viewmenuitem *item);
	HRESULT RemoveItem(const char *name, size_t positionHint);
	size_t LocateItem(const char *name, size_t start, BOOL reverse, ifc_viewmenuitem **item);
	HRESULT GetActionContext(ifc_viewaction *action, Dispatchable **context);
	HRESULT Execute(ifc_viewmenuitem *item, HWND ownerWindow);

	void OnMenuInit(int position, HWND ownerWindow);
	void OnMenuUninit(HWND ownerWindow);
	void OnMenuCommand(int position, HWND ownerWindow);
	void OnMenuSelect(int position, unsigned int flags, HWND ownerWindow);

protected:
	friend class ContextMenuEventHandler;
	friend class ContextMenuOwner;

protected:
	size_t ref;
	HMENU hMenu;
	ifc_viewactioncontextenum *contextEnum;
	ifc_viewmenugroup *root;
	BOOL extendedMode;
	ContextMenuEventHandler eventHandler;

};

#endif //_NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_HEADER
