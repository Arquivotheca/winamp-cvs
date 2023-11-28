#include <precomp.h>
#include "resource.h"
#include <commctrl.h>
#include "wa2cfgitems.h"
#include "gen.h"
#include "prefs.h"
#include "../Agave/Language/api_language.h"
#include <api/skin/skinparse.h>

void turnonoff(HWND wnd, int *t, int n, int v);
extern int toggle_from_wa2;
extern HWND subWnd;

static int ratio_all_on[] = {IDC_SLIDER_CUSTOMSCALE, IDC_STATIC_SCALE10, IDC_STATIC_SCALE300, IDC_STATIC_SCALE};
static int ratio_all_off[] = {IDC_CHECK_LINKRATIO, };
static int auto_res = -1;
static int cur_res = 10;
static DWORD cur_res_last = 0;
static int cur_res_total = 0;
static int cur_res_num = 0;
static int old_res = 0;

static const wchar_t *getScrollTextSpeedW(float v)
{
	int skipn = (int)((1.0f / v) - 1 + 0.5f);
	static wchar_t buf[16];
	ZERO(buf);
	switch (skipn)
	{
		case 0: return WASABI_API_LNGSTRINGW_BUF(IDS_FASTER,buf,16);
		case 1: return WASABI_API_LNGSTRINGW_BUF(IDS_FAST,buf,16);
		case 2: return WASABI_API_LNGSTRINGW_BUF(IDS_AVERAGE,buf,16);
		case 3: return WASABI_API_LNGSTRINGW_BUF(IDS_SLOW,buf,16);
		case 4: return WASABI_API_LNGSTRINGW_BUF(IDS_SLOWER,buf,16);
	}
	return WASABI_API_LNGSTRINGW_BUF(IDS_N_A,buf,16);
}

static void nextRes(HWND dlg)
{
	if (cur_res == 250)
	{
		cfg_uioptions_timerresolution.setValueAsInt(old_res);
		SetDlgItemTextW(dlg, IDC_TXT, WASABI_API_LNGSTRINGW(IDS_FAILED_TO_DETECT_OPTIMAL_RESOLUTION));
	}
	else
	{
		if (cur_res >= 100)
			cur_res += 5;
		else if (cur_res >= 50)
			cur_res += 2;
		else
			cur_res++;
		SetDlgItemTextW(dlg, IDC_TXT, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_AUTO_DETECTING), cur_res));
		cur_res_last = Std::getTickCount();
		cur_res_total = 0;
		cur_res_num = 0;
		SetTimer(dlg, 1, cur_res, NULL);
	}
}


static INT_PTR CALLBACK autoTimerResProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int m_count_upd, m_nb_initial_files;
	static char *m_extlist;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			old_res = cfg_uioptions_timerresolution.getValueAsInt();
			cur_res = -1;
			return 1;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			cfg_uioptions_timerresolution.setValueAsInt(old_res);
			EndDialog(hwndDlg, 0);
			return 0;
		case IDOK:
			if (cur_res == -1)
			{
				cfg_uioptions_timerresolution.setValueAsInt(250);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				SetDlgItemTextW(hwndDlg, IDC_TXT, WASABI_API_LNGSTRINGW(IDS_PREPARING_AUTO_DETECTION));
				SetTimer(hwndDlg, 2, 1000, NULL);
			}
			else EndDialog(hwndDlg, IDOK);
			return 0;
		}
		break;
	case WM_TIMER:
		{
			if (wParam == 1)
			{
				DWORD now = Std::getTickCount();
				cur_res_total += now - cur_res_last;
				cur_res_num++;
				cur_res_last = now;
				int m = 5;
				if (cur_res >= 100) m = 2;
				else if (cur_res >= 50) m = 3;
				if (cur_res_num == m)
				{
					float average = (float)cur_res_total / (float)m;
					if (average <= (float)cur_res*1.1)
					{
						auto_res = cur_res;
						cfg_uioptions_timerresolution.setValueAsInt(old_res);
						SetDlgItemTextW(hwndDlg, IDC_TXT, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_AUTO_DETECTION_SUCCESSFUL), cur_res));
						SetDlgItemTextW(hwndDlg, IDOK, WASABI_API_LNGSTRINGW(IDS_ACCEPT));
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
					}
					else
					{
						nextRes(hwndDlg);
					}
				}
				return 0;
			}
			else if (wParam == 2)
			{
				KillTimer(hwndDlg, 2);
				cur_res = 9;
				nextRes(hwndDlg);
				return 0;
			}
			break;
		}
	}
	return FALSE;
}

static void autoTimerRes(HWND dlg)
{
	INT_PTR r = WASABI_API_DIALOGBOXW(IDD_AUTOTIMERRES, dlg, autoTimerResProc);
	if (r == IDOK)
	{
		cfg_uioptions_timerresolution.setValueAsInt(auto_res);
		SendMessage(GetDlgItem(dlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETPOS, 1, auto_res);
		SetDlgItemTextW(dlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), auto_res));
	}
}

INT_PTR CALLBACK ffPrefsProc1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETRANGEMAX, 0, 250);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETRANGEMIN, 0, 10);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION), TBM_SETPOS, 1, cfg_uioptions_timerresolution.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), cfg_uioptions_timerresolution.getValueAsInt()));
			CheckDlgButton(hwndDlg, IDC_CHECK_TOOLTIPS, cfg_uioptions_tooltips.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_DOCKING, cfg_options_docking.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_DOCKING2, cfg_options_appbarondrag.getValueAsInt());
			SetDlgItemText(hwndDlg, IDC_EDIT_DOCKDISTANCE, StringPrintf("%d", cfg_options_dockingdistance.getValueAsInt()));
			SetDlgItemText(hwndDlg, IDC_EDIT_DOCKDISTANCE2, StringPrintf("%d", cfg_options_appbardockingdistance.getValueAsInt()));
			SetDlgItemText(hwndDlg, IDC_EDIT_INCREMENT, StringPrintf("%d", cfg_uioptions_textincrement.getValueAsInt()));
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALLRATIO, cfg_uioptions_linkallratio.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_RADIO_USELOCK, cfg_uioptions_uselocks.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_RADIO_ALLLOCKED, !cfg_uioptions_uselocks.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKRATIO, cfg_uioptions_linkratio.getValueAsInt());
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETRANGEMAX, 0, 4);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETRANGEMIN, 0, 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED), TBM_SETPOS, 1, (int)(1.0f / cfg_uioptions_textspeed.getValueAsDouble() - 1.0f + 0.5f));
			SetDlgItemTextW(hwndDlg, IDC_STATIC_TICKER, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TEXT_SCROLL_SPEED), getScrollTextSpeedW((float)cfg_uioptions_textspeed.getValueAsDouble())));
			Layout *main = SkinParser::getMainLayout();
			int oldscale = 1;
			if (main)
			{
				//CUT: float curratio=1;
				if (!WASABI_API_WND->rootwndIsValid(main)) { return 0; }
				oldscale = (int)main->getRenderRatio();
			}
			int u = (int)((oldscale * 100.0f) + 0.5f);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMAX, 0, 300);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETRANGEMIN, 0, 10);
			SendMessage(GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE), TBM_SETPOS, 1, u);
			SetDlgItemText(hwndDlg, IDC_STATIC_SCALE, StringPrintf("%d%%", u));
			turnonoff(hwndDlg, ratio_all_on, sizeof(ratio_all_on) / sizeof(int), cfg_uioptions_linkallratio.getValueAsInt());
			turnonoff(hwndDlg, ratio_all_off, sizeof(ratio_all_off) / sizeof(int), !cfg_uioptions_linkallratio.getValueAsInt());
			return 1;
		}
	case WM_COMMAND:
		toggle_from_wa2 = 1;
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_AUTOTIMERRES:
			autoTimerRes(hwndDlg);
			return 0;
		case IDC_CHECK_TOOLTIPS:
			cfg_uioptions_tooltips.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_TOOLTIPS));
			return 0;
		case IDC_CHECK_DOCKING:
			cfg_options_docking.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DOCKING));
			return 0;
		case IDC_CHECK_DOCKING2:
			cfg_options_appbarondrag.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DOCKING2));
			return 0;
		case IDC_EDIT_DOCKDISTANCE:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int t, a;
				a = GetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE, &t, 0);
				if (t) cfg_options_dockingdistance.setValueAsInt(a);
			}
			return 0;
		case IDC_EDIT_DOCKDISTANCE2:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int t, a;
				a = GetDlgItemInt(hwndDlg, IDC_EDIT_DOCKDISTANCE2, &t, 0);
				if (t) cfg_options_appbardockingdistance.setValueAsInt(a);
			}
			return 0;
		case IDC_EDIT_INCREMENT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int t, a;
				a = GetDlgItemInt(hwndDlg, IDC_EDIT_INCREMENT, &t, 0);
				if (t) cfg_uioptions_textincrement.setValueAsInt(a);
			}
			return 0;
		case IDC_CHECK_LINKALLRATIO:
			cfg_uioptions_linkallratio.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALLRATIO));
			turnonoff(hwndDlg, ratio_all_on, sizeof(ratio_all_on) / sizeof(int), cfg_uioptions_linkallratio.getValueAsInt());
			turnonoff(hwndDlg, ratio_all_off, sizeof(ratio_all_off) / sizeof(int), !cfg_uioptions_linkallratio.getValueAsInt());
			return 0;
		case IDC_RADIO_USELOCK:
			cfg_uioptions_uselocks.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_RADIO_USELOCK));
			return 0;
		case IDC_RADIO_ALLLOCKED:
			cfg_uioptions_uselocks.setValueAsInt(!IsDlgButtonChecked(hwndDlg, IDC_RADIO_ALLLOCKED));
			return 0;
		case IDC_CHECK_LINKRATIO:
			cfg_uioptions_linkratio.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKRATIO));
			return 0;
		}
		toggle_from_wa2 = 0;
		break;
	case WM_HSCROLL:
		{
			int t = (int)SendMessage((HWND) lParam, TBM_GETPOS, 0, 0);
			HWND ctrl = (HWND) lParam;
			if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_TIMERRESOLUTION))
			{
				cfg_uioptions_timerresolution.setValueAsInt(t);
				SetDlgItemTextW(hwndDlg, IDC_STATIC_TIMERRES, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TIMERS_RESOLUTION), cfg_uioptions_timerresolution.getValueAsInt()));
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_TICKERSPEED))
			{
				cfg_uioptions_textspeed.setValueAsDouble(1.0 / (float)(t + 1));
				SetDlgItemTextW(hwndDlg, IDC_STATIC_TICKER, StringPrintfW(WASABI_API_LNGSTRINGW(IDS_TEXT_SCROLL_SPEED), getScrollTextSpeedW((float)cfg_uioptions_textspeed.getValueAsDouble())));
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_CUSTOMSCALE))
			{
				SetDlgItemText(hwndDlg, IDC_STATIC_SCALE, StringPrintf("%d%%", t));
				Layout *main = SkinParser::getMainLayout();
				if (main)
				{
					main->setRenderRatio((float)t / 100.0);
					if (cfg_uioptions_linkratio.getValueAsInt())
					{
						int nc = SkinParser::getNumContainers();
						for (int i = 0;i < nc;i++)
						{
							Container *c = SkinParser::enumContainer(i);
							if (c)
							{
								int nl = c->getNumLayouts();
								for (int j = 0;j < nl;j++)
								{
									Layout *l = c->enumLayout(j);
									if (l)
									{
										UpdateWindow(l->gethWnd());
									}
								}
							}
						}
					}
				}
				return 0;
			}
			break;
		}
	case WM_DESTROY:
		subWnd = NULL;
		return 0;
	}

	const int controls[] = 
	{
		IDC_SLIDER_TIMERRESOLUTION,
		IDC_SLIDER_TICKERSPEED,
		IDC_SLIDER_CUSTOMSCALE,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}
