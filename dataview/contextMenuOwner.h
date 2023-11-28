#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_OWNER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_OWNER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./contextMenu.h"

class ContextMenuOwner
{
protected:
	ContextMenuOwner(ContextMenu *menu, HWND hwnd);
	~ContextMenuOwner();

public:
	static HRESULT CreateInstance(ContextMenu *menu, 
								  HWND hwnd,
								  ContextMenuOwner **instance);
public:
	size_t AddRef();
	size_t Release();
	
	HRESULT Detach();

protected:
	HRESULT Attach();
	LRESULT WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT PreviousWindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam);
		
protected:
	void OnDestroy();
	void OnMenuInit(HMENU hMenu, int position);
	void OnMenuUninit(HMENU hMenu);
	void OnMenuCommand(HMENU hMenu, int position);
	void OnMenuSelect(HMENU hMenu, int position, unsigned int flags);

private:
	friend static LRESULT CALLBACK ContextMenuOwner_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	size_t ref;
	ContextMenu *menu;
	HWND hwnd;
	WNDPROC previousWndProc;
	BOOL unicodeWindow;	
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_HEADER
