#include "Main.h"
#include "resource.h"
#include "util.h"
#include "../nu/HTMLContainer.h"
#include "AutoChar.h"
#include "../Winamp/wa_ipc.h"

#include "WMHandler.h"

class Browser : public HTMLContainer
{
public:
	Browser(HWND h) :HTMLContainer(h)  {}
	void NavigateToName(const wchar_t *url, void *postData, size_t postSize);
	STDMETHOD (GetExternal)(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch);
};


struct drm_params
{
	const wchar_t *url;
	void *postData;
	size_t postSize;
	WMHandler *callback;
	HANDLE event;
	Browser *browser;
};


BOOL CALLBACK MainDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			drm_params *params = (drm_params *)lParam;
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		params->browser = new Browser(hwndDlg);
		params->browser->setVisible(TRUE);
		params->browser->setFocus(TRUE);

		params->browser->NavigateToName(params->url, params->postData, params->postSize);
		SetEvent(params->event);
	
		}
		break;
	case WM_SIZE:
		{
			drm_params *params = (drm_params *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (params && params->browser)
				params->browser->SyncSizeToWindow(hwndDlg);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;
		}
	case WM_DESTROY:
	drm_params *params = (drm_params *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			if (params)
			{
		delete params->browser;
		params->browser = 0;
			}
		break;
	}
	return 0;
}

// ---------------------------------------------------------------
void Browser::NavigateToName(const wchar_t *url, void *postData, size_t postSize)
{
	static const char POST_DATA_PREFIX[] = "nonsilent=1&challenge=";
	static const LPWSTR POST_HEADER_DATA = L"Content-Type: application/x-www-form-urlencoded\r\n";

	if (!m_pweb) return ;
	
	long moptions = navNoReadFromCache | navNoWriteToCache | navNoHistory;
	VARIANT options;
	memset( (void*)&options, 0, sizeof(VARIANT));
	V_VT(&options) = VT_I4;
	V_I4(&options) = moptions;

	SAFEARRAY* saPostData = NULL;
	VARIANT vtPostData;
	VariantInit(&vtPostData);
	if (postData)
  {
		void* data;
    size_t prefixSize= lstrlenA(POST_DATA_PREFIX);
    saPostData = SafeArrayCreateVector(VT_UI1, 0, prefixSize + postSize + 1);
   
    if (FAILED(SafeArrayAccessData(saPostData, &data)))
			return;

    // Copy the prefix, then the data
    CopyMemory(data, POST_DATA_PREFIX, prefixSize);
    CopyMemory( ((LPBYTE)data) + prefixSize, postData, postSize /* + 1 */ );
    
    if (FAILED(SafeArrayUnaccessData(saPostData)))
			return;
    
    V_VT(&vtPostData) = VT_ARRAY | VT_UI1;
    V_ARRAY(&vtPostData) = saPostData;
  }

	VARIANT vtEmpty;
	VariantInit(&vtEmpty);
	BSTR bstrURL = SysAllocString(url);
	HRESULT hr = m_pweb->Navigate (bstrURL , &vtEmpty, &vtEmpty, &vtPostData, &vtEmpty);
	SysFreeString(bstrURL);

	if(saPostData)
  {
    SafeArrayDestroy( saPostData );
    saPostData = NULL;
  }
}

HRESULT Browser::GetExternal(IDispatch __RPC_FAR *__RPC_FAR *ppDispatch)
{
	if (NULL == ppDispatch) return E_POINTER;
	
	*ppDispatch = winampExternal;
	if (NULL != winampExternal)
		winampExternal->AddRef();
	
	return S_OK;
}


static DWORD CALLBACK GayThread(void *param)
{
	drm_params *params = (drm_params *)param; 
	DialogBoxParam(plugin.hDllInstance,MAKEINTRESOURCE(IDD_DRMBROWSER),plugin.hMainWindow,MainDialogProc,(LPARAM)params);
	params->callback->BrowserClosed();
	delete params;
	
	return 0;
}

HWND LaunchNonSilentDRMWindow(const wchar_t *url, void *postData, size_t postSize, WMHandler *callback)
{
	drm_params *params = new drm_params;
	params->url = url;
	params->postData = postData;
	params->postSize = postSize;
	params->callback = callback;
	params->event = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE t = CreateThread(0, 0, GayThread, params, 0, 0);
	WaitForSingleObject(params->event, INFINITE);
	CloseHandle(params->event);
	return 0;
}

void KillWindow()
{
//	if (browser->m_hwnd)
	//	ShowWindow(browser->m_hwnd, SW_HIDE);
}