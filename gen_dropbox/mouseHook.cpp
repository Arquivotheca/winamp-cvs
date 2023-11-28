#include "./main.h"
#include "./plugin.h"
#include "./mouseHook.h"

static MouseHook *activeHook = NULL;
static CRITICAL_SECTION lock;


static void CALLBACK UninitializeMouseHook(void)
{
	DeleteCriticalSection(&lock);
}

void InitializeMouseHook()
{
	InitializeCriticalSection(&lock);
	Plugin_RegisterUnloadCallback(UninitializeMouseHook);
}


MouseHook::MouseHook(HWND hwndOwner) : hook(NULL), hOwner(NULL)
{
	EnterCriticalSection(&lock);
	if (NULL == activeHook && NULL != hwndOwner && IsWindow(hwndOwner))
	{
		hook = SetWindowsHookEx(WH_MSGFILTER, MouseHook::HookProcReal, NULL, GetCurrentThreadId()); 
		if (NULL != hook)
		{
			hOwner = hwndOwner;
			activeHook = this;
		}
	}
	LeaveCriticalSection(&lock);
}

MouseHook::~MouseHook()
{
	if (NULL != hook)
	{
		UnhookWindowsHookEx(hook);

		EnterCriticalSection(&lock);
		if (activeHook == this)
			activeHook = NULL;
		LeaveCriticalSection(&lock);
	}
}

void MouseHook::Release()
{
	delete(this);
}

LRESULT MouseHook::CallNext(int nCode, MSG *pMsg)
{
	return CallNextHookEx(hook, nCode, 0, (LPARAM)pMsg);
}

LRESULT CALLBACK MouseHook::HookProcReal(int nCode, WPARAM wParam, LPARAM lParam)
{	
	MSG *pMsg = (MSG*)lParam;
	LRESULT r;

	if (NULL == activeHook)
		return 0;

	if (nCode < 0)
		return CallNextHookEx(activeHook->hook, nCode, wParam, lParam);

	if (WM_NCDESTROY == pMsg->message)
	{
		r = CallNextHookEx(activeHook->hook, nCode, wParam, lParam);
		activeHook->Release();
	}
	else 
		r = activeHook->HookProc(nCode, pMsg);
	
	return r; 
}