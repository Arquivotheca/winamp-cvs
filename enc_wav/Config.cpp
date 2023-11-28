#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include "Config.h"
#include "resource.h"
#include <strsafe.h>

static void ACM_gettext(HWND hwndDlg, char* tx)
{
	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	ACMFORMATTAGDETAILS aftd;
	ZeroMemory(&aftd, sizeof(aftd));
	aftd.cbStruct = sizeof(aftd);
	aftd.dwFormatTag = wc->cfg.convert_wfx.wfx.wFormatTag;
	if (!acmFormatTagDetails(0, &aftd, ACM_FORMATTAGDETAILSF_FORMATTAG))
	{
		char* p = aftd.szFormatTag;
		while (*p) *(tx++) = *(p++);
		*(tx++) = 13;
		*(tx++) = 10;
	}
	ACMFORMATDETAILS afd;
	ZeroMemory(&afd, sizeof(afd));
	afd.cbStruct = sizeof(afd);
	afd.dwFormatTag = wc->cfg.convert_wfx.wfx.wFormatTag;
	afd.pwfx = &wc->cfg.convert_wfx.wfx;
	afd.cbwfx = sizeof(wc->cfg.convert_wfx);
	if (!acmFormatDetails(0, &afd, ACM_FORMATDETAILSF_FORMAT))
	{
		char* p = afd.szFormat;
		while (*p) *(tx++) = *(p++);
	}
	*tx = 0;
}

static void ACM_choose(HWND hwndDlg, bool pcm)
{
	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	ACMFORMATCHOOSE afc;
	memset(&afc, 0, sizeof(afc));
	afc.cbStruct = sizeof(afc);
	afc.fdwStyle = ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
	afc.pwfx = &wc->cfg.convert_wfx.wfx;
	afc.cbwfx = sizeof(wc->cfg.convert_wfx);

	afc.hwndOwner = hwndDlg;

	if (!acmFormatChoose(&afc))
	{
		{
			char tmp[512];
			StringCchPrintf(tmp, 512, "%s\x0d\x0a%s", afc.szFormatTag, afc.szFormat);
			SetDlgItemText(hwndDlg, IDC_FORMAT_DESCRIPTION, tmp);

			StringCchPrintf(tmp, 512, "%d", wc->cfg.convert_wfx.wfx.cbSize);
			WritePrivateProfileString("enc_wav", "fmtsize", tmp, wc->configfile);
			WritePrivateProfileStruct("enc_wav", "fmt", &wc->cfg.convert_wfx, sizeof(wc->cfg.convert_wfx.wfx) + wc->cfg.convert_wfx.wfx.cbSize, wc->configfile);
		}
	}
}

INT_PTR WINAPI DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			if (!lParam) // this should NEVER happen
				return 0;

			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG)lParam);
			wc=(ConfigWnd*)lParam;

			char tmp[256];
			ACM_gettext(hwndDlg, tmp);
			SetDlgItemText(hwndDlg, IDC_FORMAT_DESCRIPTION, tmp);
			CheckDlgButton(hwndDlg, IDC_HEADER, wc->cfg.header);
			CheckDlgButton(hwndDlg, IDC_DO_CONVERT, wc->cfg.convert);
			SetDlgItemText(hwndDlg, IDC_EXTENSION, wc->cfg.wav_ext);
			SendDlgItemMessage(hwndDlg, IDC_EXTENSION, EM_SETLIMITTEXT, 4, 0);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CHOOSE_FORMAT:
			ACM_choose(hwndDlg, 0);
			break;
		case IDC_HEADER:
			wc->cfg.header = !!IsDlgButtonChecked(hwndDlg, IDC_HEADER);
			WritePrivateProfileString("enc_wav", "header", wc->cfg.header?"1":"0", wc->configfile);
			break;
		case IDC_DO_CONVERT:
			wc->cfg.convert = !!IsDlgButtonChecked(hwndDlg, IDC_DO_CONVERT);
			WritePrivateProfileString("enc_wav", "convert", wc->cfg.convert?"1":"0", wc->configfile);
			break;
		case IDC_EXTENSION:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				GetDlgItemText(hwndDlg, IDC_EXTENSION, wc->cfg.wav_ext, sizeof(wc->cfg.wav_ext));
				WritePrivateProfileString("enc_wav", "ext", wc->cfg.wav_ext, wc->configfile);
			}
			break;
		}
		break;
		
	}
	return 0;
}