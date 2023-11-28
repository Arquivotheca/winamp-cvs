#include "./modalSubclass.h"
#include "./wasabiApi.h"

static size_t tlsIndex = -1;

typedef struct __HOOKDATA
{
	HHOOK hHook;
	MODALSUBCLASSPROC callback;
	ULONG_PTR user;
} HOOKDATA;

static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    HOOKDATA *data = (HOOKDATA*)WASABI_API_APP->GetThreadStorage(tlsIndex);
	if (NULL == data)
		return 1;

	LRESULT result;

	switch(nCode)
	{
		case HCBT_CREATEWND:
			result = CallNextHookEx(data->hHook, nCode, wParam, lParam);
			if(0 == result)
				result = (data->callback) ? 
				data->callback((HWND)wParam, ((CBT_CREATEWND*)lParam)->lpcs, 
				((CBT_CREATEWND*)lParam)->hwndInsertAfter, data->user) : 0;
			
			WASABI_API_APP->SetThreadStorage(tlsIndex, NULL);
			UnhookWindowsHookEx(data->hHook);
			free(data);
			return result;
	}

	return CallNextHookEx(data->hHook, nCode, wParam, lParam);
}

BOOL BeginModalSubclass(MODALSUBCLASSPROC callback, ULONG_PTR user)
{
	if (NULL == WASABI_API_APP)
		return FALSE;

	if (-1 == tlsIndex)
	{
		tlsIndex = WASABI_API_APP->AllocateThreadStorage();
		if (-1 == tlsIndex) 
			return FALSE;
		WASABI_API_APP->SetThreadStorage(tlsIndex, NULL);
	}
	else
	{
		if (NULL != WASABI_API_APP->GetThreadStorage(tlsIndex))
			return FALSE;
	}

	HOOKDATA *data = (HOOKDATA*)malloc(sizeof(HOOKDATA));
	data->hHook = SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
	data->callback = callback;
	data->user = user;

	if (NULL == data->hHook)
	{
		free(data);
		return FALSE;
	}
	
	WASABI_API_APP->SetThreadStorage(tlsIndex, data);
	void *test = WASABI_API_APP->GetThreadStorage(tlsIndex);
	if (test != data)
	{
		WASABI_API_APP->SetThreadStorage(tlsIndex, NULL);
		UnhookWindowsHookEx(data->hHook);
		free(data);
		return FALSE;
	}

	return TRUE;
}

void EndModalSubclass()
{
	if (NULL == WASABI_API_APP || -1 == tlsIndex)
		return;

	HOOKDATA *data = (HOOKDATA*)WASABI_API_APP->GetThreadStorage(tlsIndex);
	if (NULL != data)
	{
		UnhookWindowsHookEx(data->hHook);
		WASABI_API_APP->SetThreadStorage(tlsIndex, NULL);
		free(data);
	}
}