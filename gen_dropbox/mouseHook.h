#ifndef NULLSOFT_DROPBOX_PLUGIN_MOUSE_HOOK_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_MOUSE_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class _declspec(novtable) MouseHook
{
protected:
	MouseHook(HWND hwndOwner);
	virtual ~MouseHook();

public:
	void Release();


protected:
	virtual LRESULT HookProc(int nCode, MSG *pMsg) = 0;
	LRESULT CallNext(int nCode, MSG *pMsg);


private:
	static LRESULT CALLBACK HookProcReal(int nCode, WPARAM wParam, LPARAM lParam);

protected:
	HWND		hOwner;
	HHOOK	hook;
};

void InitializeMouseHook();
#endif // NULLSOFT_DROPBOX_PLUGIN_MOUSE_HOOK_HEADER