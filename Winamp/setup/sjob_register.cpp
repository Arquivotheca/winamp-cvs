#include "main.h"
#include "../nu/AutoChar.h"
#include "./sjob_register.h"
#include "./httpgrab.h"

setup_job_register::setup_job_register() : ref(1), hwndHttp(NULL)
{
}

setup_job_register::~setup_job_register()
{
}

size_t setup_job_register::AddRef()
{
	return ++ref;
}

size_t setup_job_register::Release()
{
	if (1 == ref) 
	{
		delete(this);
		return 0;
	}
	return --ref;
}

HRESULT setup_job_register::Execute(HWND hwndText)
{	
	HRESULT hr;
	char data[8192];
	INT s;
	wchar_t szEmail[256];

	if (!isInetAvailable()) return S_OK;
	if (!config_newverchk2) return S_OK;

	SecureZeroMemory(data, sizeof(data));
				
	s = GetPrivateProfileInt("WinampReg", "RegDataLen", 0, INI_FILEA);
	if (s> 0)
	{
		if (GetPrivateProfileStruct("WinampReg", "RegData2", data, s, INI_FILEA))
		{
			GetMetricsValueW(data, MAKEINTRESOURCEA(METRICS_EMAIL), szEmail, sizeof(szEmail));
		}
	//	if (!*szEmail) return S_OK;
	}
		
	HWND hwndHost = BeginGrabHTTPText(hwndText, HTTPGRAB_USEWINDOWTEXT, &hwndHttp);
	hr = (SendMetrics(data, hwndHost)) ? S_OK : S_FALSE;
	hwndHttp = NULL;
	EndGrabHTTPText(hwndHost);
	
	return hr;
}



HRESULT setup_job_register::Cancel(HWND hwndText)
{

	if (hwndHttp) SendMessageW(hwndHttp, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 
							(LPARAM)GetDlgItem(hwndHttp, IDCANCEL));
	return S_OK;
}
HRESULT setup_job_register::IsCancelSupported()
{
	return S_OK;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS setup_job_register
START_DISPATCH
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(API_SETUPJOB_EXECUTE, Execute)
CB(API_SETUPJOB_CANCEL, Cancel)
CB(API_SETUPJOB_ISCANCELSUPPORTED, IsCancelSupported)
END_DISPATCH
#undef CBCLASS