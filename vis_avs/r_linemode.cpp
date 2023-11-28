#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

int g_line_blend_mode;

static int line_blendmodes[]= 
{
	IDS_REPLACE,
	IDS_ADDITIVE,
	IDS_MAXIMUM_BLEND,
	IDS_50_50_BLEND,
	IDS_SUBTRACTIVE_BLEND_1,
	IDS_SUBTRACTIVE_BLEND_2,
	IDS_MULTIPLY_BLEND,
	IDS_ADJUSTABLE_BLEND,
	IDS_XOR,
	IDS_MINIMUM_BLEND,
};

#define C_THISCLASS C_LineModeClass
#define MOD_NAME "Misc / Set render mode"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_MISC_SET_RENDER_MODE,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);
		int newmode;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { newmode=GET_INT(); pos+=4; }
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(newmode); pos+=4;
	return pos;
}

C_THISCLASS::C_THISCLASS()
{
  newmode=0x80010000;
}

C_THISCLASS::~C_THISCLASS()
{
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	if (isBeat&0x80000000) return 0;
	if (newmode&0x80000000)
	{
	    g_line_blend_mode=newmode&0x7fffffff;
	}
	return 0;
}

C_RBASE *R_LineMode(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}

static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				for (int x = 0; x<sizeof(line_blendmodes)/sizeof(line_blendmodes[0]); x ++)
				{
					SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)WASABI_API_LNGSTRING(line_blendmodes[x]));
				}
			}
			if (g_this->newmode&0x80000000)
				CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
			SendDlgItemMessage(hwndDlg, IDC_ALPHASLIDE, TBM_SETRANGE, TRUE, MAKELONG(0, 255));
  			SendDlgItemMessage(hwndDlg, IDC_ALPHASLIDE, TBM_SETPOS, TRUE, (int)(g_this->newmode>>8)&0xff);      
			SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SETCURSEL,(WPARAM)g_this->newmode&0xff,0);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OUTSLIDE), (g_this->newmode&0xff) == 7);
			SetDlgItemInt(hwndDlg,IDC_EDIT1,(g_this->newmode>>16)&0xff,FALSE);
			return 1;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_EDIT1:
				{
					int r,t;
					r=GetDlgItemInt(hwndDlg,IDC_EDIT1,&t,FALSE);
					if (t)
					{
						g_this->newmode&=~0xff0000;
						g_this->newmode|=(r&0xff)<<16;
					}
				}
					break;

				case IDC_CHECK1:
					g_this->newmode&=0x7fffffff;
					g_this->newmode|=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?0x80000000:0;
					break;

				case IDC_COMBO1:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						int r=SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
						if (r!=CB_ERR) 
						{
							g_this->newmode&=~0xff;
							g_this->newmode|=r;
							EnableWindow(GetDlgItem(hwndDlg, IDC_OUTSLIDE), r == 7);
						}
					}
					break;
			}
			return 0;

		case WM_NOTIFY:
			if (LOWORD(wParam) == IDC_ALPHASLIDE)
			{
				g_this->newmode &= ~0xff00;
				g_this->newmode |= (SendDlgItemMessage(hwndDlg, IDC_ALPHASLIDE, TBM_GETPOS, 0, 0)&0xff)<<8;
			}
			break;
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_LINEMODE,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_LineMode(char *desc)
{
  return NULL;
}
#endif