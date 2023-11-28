#include <precomp.h>
#include "resource.h"
#include "prefs.h"
#include "wa2cfgitems.h"
#include <bfc/std_wnd.h>
#include "../Agave/Language/api_language.h"
#include <api/skin/skinparse.h>
#include <commctrl.h>
void turnonoff(HWND wnd, int *t, int n, int v);
extern int toggle_from_wa2;
int opacity_all_on[] = {IDC_SLIDER_CUSTOMALPHA, IDC_STATIC_TRANSP, IDC_STATIC_ALPHA, IDC_STATIC_OPAQUE, IDC_RADIO_AUTO100_HOVER, IDC_RADIO_AUTO100_FOCUS, IDC_RADIO_NOAUTO100};
int opacity_all_off[] = {IDC_CHECK_LINKALPHA, };
int opacity_unavail[] = {IDC_STATIC_OPACITY, IDC_CHECK_LINKALLALPHA, IDC_RADIO_AUTO100_FOCUS, IDC_RADIO_AUTO100_HOVER, IDC_RADIO_NOAUTO100,
                            IDC_SLIDER_CUSTOMALPHA,IDC_STATIC_TRANSP,IDC_STATIC_ALPHA,IDC_STATIC_OPAQUE,IDC_CHECK_LINKALPHA,IDC_STATIC_AUTOON,
                            IDC_STATIC_AUTOONTXT,IDC_STATIC_FADEIN2,IDC_SLIDER_FADEIN,IDC_STATIC_FADEIN,IDC_STATIC_HOLD2,IDC_SLIDER_HOLD,
                            IDC_STATIC_HOLD,IDC_STATIC_FADEOUT2,IDC_SLIDER_FADEOUT,IDC_STATIC_FADEOUT,IDC_STATIC_EXTENDBOX,IDC_EDIT_EXTEND,
                            IDC_STATIC_EXTEND};
int desktopalpha_unavail[] = {IDC_STATIC_DA1, IDC_STATIC_DA2, IDC_STATIC_DA3, IDC_CHECK_DESKTOPALPHA};

static UINT get_logslider(HWND wnd) {
	int z = SendMessage(wnd, TBM_GETPOS, 0, 0);
	long a = (long)(0.5 + 303.03 * pow(100.0, (double)z/30303.0) - 303.03);
	return a;
}

void set_logslider(HWND wnd, int b) {
	long a = (long) (0.5 + 30303.0 * log((double)(b+303.0)/303.03)/log(100.0));
	SendMessage(wnd, TBM_SETPOS, 1, a);
}

INT_PTR CALLBACK ffPrefsProc4(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) {
	switch (uMsg) {
		case WM_INITDIALOG:
		{
			CheckDlgButton(hwndDlg, IDC_CHECK_DESKTOPALPHA, cfg_uioptions_desktopalpha.getValueAsInt());
			int transpavail = StdWnd::isTransparencyAvailable();
			if (!StdWnd::isDesktopAlphaAvailable()) turnonoff(hwndDlg, desktopalpha_unavail, sizeof(desktopalpha_unavail)/sizeof(int), 0);
			if (!transpavail) turnonoff(hwndDlg, opacity_unavail, sizeof(opacity_unavail)/sizeof(int), 0);
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALPHA, cfg_uioptions_linkalpha.getValueAsInt());
			CheckDlgButton(hwndDlg, IDC_CHECK_LINKALLALPHA, cfg_uioptions_linkallalpha.getValueAsInt());
			Layout *main = SkinParser::getMainLayout();
			int curalpha = 255;
			int auto100_hover = 0;
			int auto100_focus = 0;
			if (main) {
				//CUT: float curratio=1;
				if (!WASABI_API_WND->rootwndIsValid(main)) { return 0; }
				curalpha = cfg_uioptions_linkedalpha.getValueAsInt();
				auto100_hover = cfg_uioptions_autoopacitylinked.getValueAsInt() == 1;
				auto100_focus = cfg_uioptions_autoopacitylinked.getValueAsInt() == 2;
			}
			int v = (int)((curalpha / 255.0f * 100.0f)+0.5f);
			wchar_t msStr[16] = {0};
			WASABI_API_LNGSTRINGW_BUF(IDS_MS,msStr,16);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETRANGEMAX,0,100);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETRANGEMIN,0,10);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA),TBM_SETPOS,1,v);
			CheckDlgButton(hwndDlg, IDC_RADIO_AUTO100_HOVER, auto100_hover);
			CheckDlgButton(hwndDlg, IDC_RADIO_AUTO100_FOCUS, auto100_focus);
			CheckDlgButton(hwndDlg, IDC_RADIO_NOAUTO100, auto100_focus == 0 && auto100_hover == 0);
			SetDlgItemText(hwndDlg, IDC_STATIC_ALPHA, StringPrintf("%d%%", v));
			turnonoff(hwndDlg, opacity_all_on, sizeof(opacity_all_on)/sizeof(int), cfg_uioptions_linkallalpha.getValueAsInt() && transpavail);
			turnonoff(hwndDlg, opacity_all_off, sizeof(opacity_all_off)/sizeof(int), !cfg_uioptions_linkallalpha.getValueAsInt() && transpavail);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),TBM_SETRANGEMAX,0,30303);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_HOLD),cfg_uioptions_autoopacitytime.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_HOLD, StringPrintfW(L"%d%s", cfg_uioptions_autoopacitytime.getValueAsInt(),msStr));
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),TBM_SETRANGEMAX,0,30303);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_FADEIN),cfg_uioptions_autoopacityfadein.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_FADEIN, StringPrintfW(L"%d%s", cfg_uioptions_autoopacityfadein.getValueAsInt(),msStr));
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),TBM_SETRANGEMAX,0,30303);
			SendMessage(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),TBM_SETRANGEMIN,0,0);
			set_logslider(GetDlgItem(hwndDlg,IDC_SLIDER_FADEOUT),cfg_uioptions_autoopacityfadeout.getValueAsInt());
			SetDlgItemTextW(hwndDlg, IDC_STATIC_FADEOUT, StringPrintfW(L"%d%s", cfg_uioptions_autoopacityfadeout.getValueAsInt(),msStr));
			SetDlgItemText(hwndDlg, IDC_EDIT_EXTEND, StringPrintf("%d", cfg_uioptions_extendautoopacity.getValueAsInt()));
			return 1;
		}
		case WM_HSCROLL:
		{
			char msStr[16] = {0};
			WASABI_API_LNGSTRING_BUF(IDS_MS,msStr,16);
			HWND ctrl = (HWND)lParam;
			int t=(int)SendMessage((HWND) lParam,TBM_GETPOS,0,0);
			if (ctrl == GetDlgItem(hwndDlg,IDC_SLIDER_CUSTOMALPHA)) {
				SetDlgItemText(hwndDlg, IDC_STATIC_ALPHA, StringPrintf("%d%%", t));
				int v = (int)(t / 100.0 * 255 + 0.5);
				if (v == 254) v = 255;
				cfg_uioptions_linkedalpha.setValueAsInt(v);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_HOLD)) {
				t = get_logslider(ctrl);
				SetDlgItemText(hwndDlg, IDC_STATIC_HOLD, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacitytime.setValueAsInt(t);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_FADEIN)) {
				t = get_logslider(ctrl);
				SetDlgItemText(hwndDlg, IDC_STATIC_FADEIN, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacityfadein.setValueAsInt(t);
			}
			else if (ctrl == GetDlgItem(hwndDlg, IDC_SLIDER_FADEOUT)) {
				int t = get_logslider(ctrl);
				SetDlgItemText(hwndDlg, IDC_STATIC_FADEOUT, StringPrintf("%d%s", t, msStr));
				cfg_uioptions_autoopacityfadeout.setValueAsInt(t);
			}
			break;
		}
		case WM_COMMAND:
			toggle_from_wa2 = 1;
			switch (LOWORD(wParam)) {
				case IDC_CHECK_DESKTOPALPHA:
					cfg_uioptions_desktopalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DESKTOPALPHA));
				return 0;
				case IDC_CHECK_LINKALPHA:
					cfg_uioptions_linkalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALPHA));
				return 0;
				case IDC_CHECK_LINKALLALPHA: 
					cfg_uioptions_linkallalpha.setValueAsInt(IsDlgButtonChecked(hwndDlg, IDC_CHECK_LINKALLALPHA));
					turnonoff(hwndDlg, opacity_all_on, sizeof(opacity_all_on)/sizeof(int), cfg_uioptions_linkallalpha.getValueAsInt());
					turnonoff(hwndDlg, opacity_all_off, sizeof(opacity_all_off)/sizeof(int), !cfg_uioptions_linkallalpha.getValueAsInt());
				return 0;
				case IDC_EDIT_EXTEND:
					if (HIWORD(wParam) == EN_CHANGE) {
						int t,a;
						char msStr[16] = {0};
						a=GetDlgItemInt(hwndDlg,IDC_EDIT_EXTEND,&t,0);
						if (t) cfg_uioptions_extendautoopacity.setValueAsInt(MAX(a,0));
						if (a < 0) SetDlgItemText(hwndDlg, IDC_STATIC_HOLD,
												  StringPrintf("%d%s", cfg_uioptions_autoopacitytime.getValueAsInt(),WASABI_API_LNGSTRING_BUF(IDS_MS,msStr,16)));
					}
				return 0;
				case IDC_RADIO_AUTO100_HOVER: 
					cfg_uioptions_autoopacitylinked.setValueAsInt(1);
				return 0;
				case IDC_RADIO_AUTO100_FOCUS: 
					cfg_uioptions_autoopacitylinked.setValueAsInt(2);
				return 0;
				case IDC_RADIO_NOAUTO100: 
					cfg_uioptions_autoopacitylinked.setValueAsInt(0);
				return 0;
			}
	}

	const int controls[] = 
	{
		IDC_SLIDER_CUSTOMALPHA,
		IDC_SLIDER_HOLD,
		IDC_SLIDER_FADEIN,
		IDC_SLIDER_FADEOUT,
	};
	if (FALSE != WASABI_API_APP->DirectMouseWheel_ProcessDialogMessage(hwndDlg, uMsg, wParam, lParam, controls, ARRAYSIZE(controls)))
		return TRUE;

	return 0;
}