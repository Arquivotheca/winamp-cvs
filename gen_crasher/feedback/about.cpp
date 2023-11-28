#include ".\main.h"

BOOL CALLBACK AboutDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				EndDialog(hwndDlg, TRUE);
				break;
			}
			break;
	}
	return FALSE;
}