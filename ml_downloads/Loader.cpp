#include "main.h"
#include "api.h"
#include "../winamp/wa_ipc.h"
#include "DownloadStatus.h"

using namespace Nullsoft::Utility;
static WNDPROC wa_oldWndProc=0;

/* protocol must be all lower case */
bool ProtocolMatch(const char *file, const char *protocol)
{
	size_t protSize = strlen(protocol);
	for (size_t i=0;i!=protSize;i++)
	{
		if (!file[i] 
		|| tolower(file[i]) != protocol[i])
			return false;
	}
	return true;
}

LRESULT CALLBACK LoaderProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_WA_IPC)
	{
		if (lParam == DOWNLOADS_UPDATE)
		{
			Navigation_Update();
		}
		else if (lParam == DOWNLOADS_VIEW_LOADED)
		{
			return 1;
		}
	}

	if (wa_oldWndProc)
		return CallWindowProc(wa_oldWndProc, hwnd, uMsg, wParam, lParam);
	else
		return 0;
}

void BuildLoader(HWND winampWindow)
{
	if (IsWindowUnicode(winampWindow))
		wa_oldWndProc=(WNDPROC) SetWindowLongPtrW(winampWindow,GWLP_WNDPROC,(LONG_PTR)LoaderProc);
	else
		wa_oldWndProc=(WNDPROC) SetWindowLongPtrA(winampWindow,GWLP_WNDPROC,(LONG_PTR)LoaderProc);
}

void DestroyLoader(HWND winampWindow)
{
	//if (wa_oldWndProc)
	//	SetWindowLong(winampWindow,GWL_WNDPROC,(LONG)wa_oldWndProc);
	//wa_oldWndProc=0;
}