#include ".\main.h"
#include <commctrl.h>

BOOL CALLBACK SilentDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			HWND hwndPrg;
            hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
			SendMessage(hwndPrg, PBM_SETRANGE, 0, MAKELPARAM(0,100));
			SendMessage(hwndPrg, PBM_SETPOS, 0, 0);
			SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Starting reporter...");
			UpdateWindow(hwndDlg);
			if ((settings.createLOG && !settings.ReadLogCollectResult())  &&
				(settings.createDMP && !settings.ReadDmpCollectResult()) ) 
			{
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Data was not generated.");
				SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
				UpdateWindow(hwndDlg);
				SetTimer(hwndDlg, 126, 2000, NULL);
				break;
			}
			SetTimer(hwndDlg, 123, 500, NULL);
			break;
			
		case WM_TIMER:
			if (wParam == 123)
			{
				KillTimer(hwndDlg, wParam);
				HWND hwndPrg;
				hwndPrg = GetDlgItem(hwndDlg, IDC_PRG_COLLECT);
				SendMessage(hwndPrg, PBM_SETPOS, 20, 0);
				if (settings.zipData)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Packing results...");		
					if(!ZipData())
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to pack results.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						UpdateWindow(hwndDlg);
						SetTimer(hwndDlg, 126, 2000, NULL);
						break;
					}
				}
				SendMessage(hwndPrg, PBM_SETPOS, 40, 0);
				UpdateWindow(hwndDlg);
				if (settings.sendData)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Sending results...");
					UpdateWindow(hwndDlg);
					if(!SendData(hwndDlg))
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to send data.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						UpdateWindow(hwndDlg);
						SetTimer(hwndDlg, 126, 2000, NULL);
						break;
					}
				}
				if (settings.autoRestart)
				{
					SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Restarting Winamp...");		
					SendMessage(hwndPrg, PBM_SETPOS, 80, 0);
					UpdateWindow(hwndDlg);
					if(!Restart())
					{
						SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Error. Unable to restart Winamp.");
						SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
						UpdateWindow(hwndDlg);
						SetTimer(hwndDlg, 126, 2000, NULL);
						break;
					}
				}
				SetDlgItemText(hwndDlg, IDC_LBL_STEP, L"Done.");		
				SendMessage(hwndPrg, PBM_SETPOS, 100, 0);
				UpdateWindow(hwndDlg);
				SetTimer(hwndDlg, 126, 1000, NULL);
				
			}
			else if (wParam == 126)
			{
				KillTimer(hwndDlg, wParam);
				EndDialog(hwndDlg, TRUE);

			}
			break;
	}
	return FALSE;
}