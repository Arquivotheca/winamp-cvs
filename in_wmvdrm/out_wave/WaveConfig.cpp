#include "WaveConfig.h"
#include "api.h"
#include <commctrl.h>
#include <math.h>
#include "output/OutWave.h"
#include "res_wav/resource.h"
#include "../nu/Config.h"
#include "../nu/AutoWide.h"
#include <strsafe.h>
#pragma warning (disable:4800)

Nullsoft::Utility::Config waveConfig;
using namespace out_wave;
unsigned int out_wave::cfg_dev;
int out_wave::cfg_buf_ms, out_wave::cfg_prebuf, out_wave::cfg_trackhack;
bool out_wave::cfg_volume, out_wave::cfg_altvol, out_wave::cfg_resetvol;
extern Out_Module waveMod;

#define load_var(V) V=waveConfig.cfg_int(L#V, V)
#define save_var(V) waveConfig.cfg_int(L#V, V)=V

void SetDefaults()
{
	cfg_dev = 0;
	cfg_buf_ms = 2000;
	cfg_prebuf = 200;
	cfg_trackhack = 200;
	cfg_volume = 1;
	cfg_altvol = 0;
	cfg_resetvol = 0;
}

void SaveConfig()
{
	save_var(cfg_buf_ms);
	save_var(cfg_dev);
	save_var(cfg_volume);
	save_var(cfg_altvol);
	save_var(cfg_resetvol);
	save_var(cfg_prebuf);
	save_var(cfg_trackhack);
}

void LoadConfig(LPCTSTR iniFile)
{
	SetDefaults();
	waveConfig.SetFile(iniFile, L"out_wave");
	load_var(cfg_buf_ms);
	load_var(cfg_dev);
	load_var(cfg_volume);
	load_var(cfg_altvol);
	load_var(cfg_resetvol);
	load_var(cfg_prebuf);
	load_var(cfg_trackhack);
}
#define BUFFER_SCALE 4000.0

static UINT cur_buffer;

static UINT get_buffer(HWND wnd)
{
	if (cur_buffer) return cur_buffer;

	//0-BUFFER_SCALE => 200-20000
	intptr_t z = SendDlgItemMessage(wnd, IDC_BUFFER, TBM_GETPOS, 0, 0);
	return cur_buffer = (UINT) ( 0.5 + 200.0 * pow(100.0, (double)z / BUFFER_SCALE) );

}

static void set_buffer(HWND wnd, UINT b)
{
	cur_buffer = b;
	SendDlgItemMessage(wnd, IDC_BUFFER, TBM_SETPOS, 1, (long) ( 0.5 + BUFFER_SCALE * log( (double)b / 200.0 ) / log( 100.0 ) ));
}

static void update_prebuf_1(HWND wnd)
{
	wchar_t zz[128] = {0};
	StringCchPrintfW(zz ,128, WASABI_API_LNGSTRINGW_WAV(IDS_WAVE_U_MS)/*L"%u ms"*/, SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_GETPOS, 0, 0));
	SetDlgItemText(wnd, IDC_PREBUF_DISP_1, zz);
}

static void update_prebuf_2(HWND wnd)
{
	wchar_t zz[128] = {0};
	StringCchPrintfW(zz ,128, WASABI_API_LNGSTRINGW_WAV(IDS_WAVE_U_MS)/*L"%u ms"*/, SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_GETPOS, 0, 0));
	SetDlgItemText(wnd, IDC_PREBUF_DISP_2, zz);
}

static void update_prebuf_range(HWND wnd)
{
	UINT max = get_buffer(wnd);
	if (max > 0x7FFF) max = 0x7FFF;
	SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_SETRANGE, 1, MAKELONG(0, max));
	SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_SETRANGE, 1, MAKELONG(0, max));
}


static void update_buf(HWND wnd)
{
	wchar_t zz[128] = {0};
	StringCchPrintfW(zz ,128, WASABI_API_LNGSTRINGW_WAV(IDS_WAVE_U_MS)/*L"%u ms"*/, get_buffer(wnd));
	SetDlgItemText(wnd, IDC_BUF_DISP, zz);
}

static UINT atoui(wchar_t* s)
{
	int ret = 0;
	while (*s >= '0' && *s <= '9') {ret = 10 * ret + (*s - '0');s++;}
	return ret;
}

BOOL WINAPI WaveCfgProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			wchar_t title[128] = {0}, temp[128] = {0};
			StringCchPrintfW(title,128,WASABI_API_LNGSTRINGW_WAV(IDS_PREFS_TITLE),WASABI_API_LNGSTRINGW_BUF_WAV(IDS_NULLSOFT_WAVEOUT_OLD,temp,128));
			SetWindowTextW(wnd,title);

			SendDlgItemMessage(wnd, IDC_VOL_ENABLE, BM_SETCHECK, (long)cfg_volume, 0);
			SendDlgItemMessage(wnd, IDC_ALT_VOL, BM_SETCHECK, (long)cfg_altvol, 0);
			SendDlgItemMessage(wnd, IDC_VOL_RESET, BM_SETCHECK, (long)cfg_resetvol, 0);

			{
				int dev;
				HWND w = GetDlgItem(wnd, IDC_DEV);
				UINT max = waveOutGetNumDevs();
				WAVEOUTCAPS caps;
				for (dev = -1;dev < (int)max;dev++)
				{
					waveOutGetDevCaps((UINT)dev, &caps, sizeof(caps));
					SendMessage(w, CB_ADDSTRING, 0, (LPARAM)caps.szPname);
				}
				SendMessage(w, CB_SETCURSEL, cfg_dev, 0);
			}

			SendDlgItemMessage(wnd, IDC_BUFFER, TBM_SETRANGE, 0, MAKELONG(0, (int)BUFFER_SCALE));
			set_buffer(wnd, cfg_buf_ms);
			update_prebuf_range(wnd);
			SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_SETPOS, 1, cfg_prebuf);
			SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_SETPOS, 1, cfg_trackhack);
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			update_buf(wnd);

			SetTimer(wnd, 1, 500, 0);
			WaveCfgProc(wnd, WM_TIMER, 0, 0);
		}
		return 1;
	case WM_COMMAND:
		switch (wp)
		{
		case IDC_RESET:
			SendDlgItemMessage(wnd, IDC_VOL_ENABLE, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(wnd, IDC_ALT_VOL, BM_SETCHECK, 0, 0);
			SendDlgItemMessage(wnd, IDC_VOL_RESET, BM_SETCHECK, 0, 0);

			SendDlgItemMessage(wnd, IDC_DEV, CB_SETCURSEL, 0, 0);

			set_buffer(wnd, 2000);
			update_prebuf_range(wnd);
			SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_SETPOS, 1, 200);
			SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_SETPOS, 1, 200);
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			update_buf(wnd);
			break;
		case IDOK:
			KillTimer(wnd, 1);
			cfg_dev = SendDlgItemMessage(wnd, IDC_DEV, CB_GETCURSEL, 0, 0);
			cfg_buf_ms = get_buffer(wnd);
			cfg_prebuf = SendDlgItemMessage(wnd, IDC_PREBUFFER_1, TBM_GETPOS, 0, 0);
			cfg_trackhack = SendDlgItemMessage(wnd, IDC_PREBUFFER_2, TBM_GETPOS, 0, 0);
			cfg_volume = (bool)SendDlgItemMessage(wnd, IDC_VOL_ENABLE, BM_GETCHECK, 0, 0);
			cfg_altvol = (bool)SendDlgItemMessage(wnd, IDC_ALT_VOL, BM_GETCHECK, 0, 0);
			cfg_resetvol = (bool)SendDlgItemMessage(wnd, IDC_VOL_RESET, BM_GETCHECK, 0, 0);
			EndDialog(wnd, 1);
			break;
		case IDCANCEL:
			KillTimer(wnd, 1);
			EndDialog(wnd, 0);
			break;
		}
		break;
	case WM_HSCROLL:
		switch (GetWindowLong((HWND)lp, GWL_ID))
		{
		case IDC_BUFFER:
			cur_buffer = 0;
			update_buf(wnd);
			update_prebuf_range(wnd);
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			break;
		case IDC_PREBUFFER_1:
			update_prebuf_1(wnd);
			break;
		case IDC_PREBUFFER_2:
			update_prebuf_2(wnd);
			break;
		}
		break;
	case WM_TIMER:
		{
			TCHAR poo[512];
			bool z = wave.get_waveout_state(poo, 512);
			SetDlgItemText(wnd, IDC_STATE, z ? poo : WASABI_API_LNGSTRINGW_WAV(IDS_NOT_ACTIVE));
			EnableWindow(GetDlgItem(wnd, IDC_BLAH), z);
		}
		break;
	}

	const int controls[] = 
	{
		IDC_BUFFER,
		IDC_PREBUFFER_1,
		IDC_PREBUFFER_2,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(wnd, msg, wp, lp, controls, ARRAYSIZE(controls)))
	{
		return TRUE;
	}
	return 0;
}