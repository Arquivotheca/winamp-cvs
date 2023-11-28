#include "main.h"
#include "./contextMenuOwner.h"

#define CONTEXTMENUOWNER_WINDOW_PROP		L"ContextMenuOwner"


static unsigned int CONTEXTMENUOWNER_WM_DETACH = 0;


ContextMenuOwner::ContextMenuOwner(ContextMenu *_menu, HWND _hwnd)
	: ref(1), menu(_menu), hwnd(_hwnd), previousWndProc(NULL), unicodeWindow(FALSE)
{
	if (NULL != menu)
		menu->AddRef();

	if (0 == CONTEXTMENUOWNER_WM_DETACH)
		CONTEXTMENUOWNER_WM_DETACH = RegisterWindowMessage(L"ContextMenuOwner_Detach");
}

ContextMenuOwner::~ContextMenuOwner()
{
	SafeRelease(menu);
	Detach();
}


HRESULT ContextMenuOwner::CreateInstance(ContextMenu *menu, HWND hwnd, ContextMenuOwner **instance)
{
	ContextMenuOwner *self;
	HRESULT hr;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == menu || NULL == hwnd)
		return E_INVALIDARG;


	self = new (std::nothrow) ContextMenuOwner(menu, hwnd);
	if (NULL == self)
		return E_OUTOFMEMORY;

	hr = self->Attach();
	if (FAILED(hr))
	{
		self->Release();
		return hr;
	}

	*instance = self;
	return S_OK;
}

size_t ContextMenuOwner::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ContextMenuOwner::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT ContextMenuOwner::Attach()
{
	if (NULL == hwnd)
		return E_UNEXPECTED;

	if (NULL != previousWndProc ||
		NULL != GetProp(hwnd, CONTEXTMENUOWNER_WINDOW_PROP))
	{
		return E_FAIL;
	}
	
	unicodeWindow = (FALSE != IsWindowUnicode(hwnd));

	SetLastError(ERROR_SUCCESS);
	previousWndProc = (WNDPROC)(LONG_PTR)((FALSE != unicodeWindow) ? 
							SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ContextMenuOwner_WindowProc) :
							SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)ContextMenuOwner_WindowProc));
	
							if (NULL == previousWndProc)
	{
		unsigned long errorCode;
		errorCode = GetLastError();
		if (ERROR_SUCCESS != errorCode)
			return HRESULT_FROM_WIN32(errorCode);
	}
	
	if (FALSE == SetProp(hwnd, CONTEXTMENUOWNER_WINDOW_PROP, this))
	{
		if (FALSE != unicodeWindow)
			SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
		else
			SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);

		return E_FAIL;
	}
	
	AddRef();

	return S_OK;

}

HRESULT ContextMenuOwner::Detach()
{
	ContextMenuOwner *attachedInstance;

	if (NULL == hwnd)
		return S_FALSE;
		
	attachedInstance = (ContextMenuOwner*)GetProp(hwnd, CONTEXTMENUOWNER_WINDOW_PROP);
	if (attachedInstance == this)
		RemoveProp(hwnd, CONTEXTMENUOWNER_WINDOW_PROP);
	
	if (NULL != previousWndProc)
	{
		if (FALSE != unicodeWindow)
		{
			if ((LONGX86)(LONG_PTR)ContextMenuOwner_WindowProc == GetWindowLongPtrW(hwnd, GWLP_WNDPROC))
				SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
		}
		else
		{
			if ((LONGX86)(LONG_PTR)ContextMenuOwner_WindowProc == GetWindowLongPtrA(hwnd, GWLP_WNDPROC))
				SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)previousWndProc);
		}
		previousWndProc = NULL;
	}

	hwnd = NULL;

	if (attachedInstance == this)
	{
		Release();
	}
	
	return S_OK;
}


void ContextMenuOwner::OnDestroy()
{
	WNDPROC previousWndProcCopy;
	BOOL unicodeWindowCopy;

	previousWndProcCopy = previousWndProc;
	unicodeWindowCopy = unicodeWindow;

	Detach();

	if (NULL != previousWndProcCopy)
	{
		if (FALSE != unicodeWindowCopy)
			CallWindowProcW(previousWndProcCopy, hwnd, WM_DESTROY, 0, 0L);
		else
			CallWindowProcA(previousWndProcCopy, hwnd, WM_DESTROY, 0, 0L);
	}
}

void ContextMenuOwner::OnMenuInit(HMENU hMenu, int position)
{
	ContextMenu *targetMenu;

	if (SUCCEEDED(ContextMenu::GetFromHandle(hMenu, &targetMenu)))
	{
		targetMenu->OnMenuInit(position, hwnd);
		targetMenu->Release();
	}
}

void ContextMenuOwner::OnMenuUninit(HMENU hMenu)
{
	ContextMenu *targetMenu;

	if (SUCCEEDED(ContextMenu::GetFromHandle(hMenu, &targetMenu)))
	{
		targetMenu->OnMenuUninit(hwnd);
		targetMenu->Release();
	}

	if (menu->GetHandle() == hMenu)
	{
		if (0 != CONTEXTMENUOWNER_WM_DETACH)
		{
			PostMessage(hwnd, CONTEXTMENUOWNER_WM_DETACH, 2, 0L);
		}
		else
			Detach();
	}
}

void ContextMenuOwner::OnMenuCommand(HMENU hMenu, int position)
{
	ContextMenu *targetMenu;

	if (SUCCEEDED(ContextMenu::GetFromHandle(hMenu, &targetMenu)))
	{
		targetMenu->OnMenuCommand(position, hwnd);
		targetMenu->Release();
	}
}

void ContextMenuOwner::OnMenuSelect(HMENU hMenu, int position, unsigned int flags)
{
	ContextMenu *targetMenu;

	if (SUCCEEDED(ContextMenu::GetFromHandle(hMenu, &targetMenu)))
	{
		targetMenu->OnMenuSelect(position, flags, hwnd);
		targetMenu->Release();
	}
}

LRESULT ContextMenuOwner::WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			OnDestroy();
			return 0;

		case WM_INITMENUPOPUP:
			OnMenuInit((HMENU)wParam, LOWORD(lParam));
			return 0;

		case WM_UNINITMENUPOPUP:
			OnMenuUninit((HMENU)wParam);
			return 0;

		case WM_MENUCOMMAND:
			OnMenuCommand((HMENU)lParam, (int)wParam);
			return 0;

		case WM_MENUSELECT:
			OnMenuSelect((HMENU)lParam, (int)LOWORD(wParam), HIWORD(wParam));
			return 0;
	}

	if (CONTEXTMENUOWNER_WM_DETACH == uMsg && 
		0 != CONTEXTMENUOWNER_WM_DETACH)
	{
		if (wParam > 0)
		{
			PostMessage(hwnd, CONTEXTMENUOWNER_WM_DETACH, (WPARAM)(wParam-1), 0L);
		}
		else
		{
			Detach();
		}
		
		return 0;
	}

	return PreviousWindowProc(uMsg, wParam, lParam);
}

LRESULT ContextMenuOwner::PreviousWindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	return (FALSE != unicodeWindow) ? 
			CallWindowProcW(previousWndProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(previousWndProc, hwnd, uMsg, wParam, lParam);

}
		
static LRESULT CALLBACK 
ContextMenuOwner_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ContextMenuOwner *self;

	self = (ContextMenuOwner*)GetProp(hwnd, CONTEXTMENUOWNER_WINDOW_PROP);
	if (NULL == self)
	{
		return (FALSE != IsWindowUnicode(hwnd)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return self->WindowProc(uMsg, wParam, lParam);
}
