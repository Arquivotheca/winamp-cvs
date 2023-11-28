#include "main.h"
#include "api.h"
#include "resource.h"

bool config_show_average_bitrate = true;

INT_PTR CALLBACK ConfigProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			char exts[1024]="";
			GetPrivateProfileStringA("in_mp4", "extensionlist", defaultExtensions, exts, 1024, m_ini);
			SetDlgItemTextA(hwndDlg, IDC_EXTENSIONLIST, exts);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_DEFAULT:
			SetDlgItemTextA(hwndDlg, IDC_EXTENSIONLIST, defaultExtensions);
			break;
		case IDOK:
			{
				char exts[1024];
				GetDlgItemTextA(hwndDlg, IDC_EXTENSIONLIST, exts, 1024);
				if (!_stricmp(exts, defaultExtensions)) // same as default?
					WritePrivateProfileStringA("in_mp4", "extensionlist", 0, m_ini); 
				else
					WritePrivateProfileStringA("in_mp4", "extensionlist", exts, m_ini);
				free(mod.FileExtensions);
				mod.FileExtensions = BuildExtensions(exts);
				EndDialog(hwndDlg, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 1);
			break;
		}
		break;
	}
	return 0;
}
void config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_CONFIG, hwndParent, ConfigProc);
}