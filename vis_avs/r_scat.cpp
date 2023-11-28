// alphachannel safe 11/21/99
#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "timing.h"
#include "../Agave/Language/api_language.h"

#ifndef LASER

#define C_THISCLASS C_ScatClass
#define MOD_NAME "Trans / Scatter"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_SCATTER,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		int enabled;
		int fudgetable[512],ftw;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
	return pos;
}

C_THISCLASS::C_THISCLASS()
{
	enabled=1;
	ftw=0;
}

C_THISCLASS::~C_THISCLASS()
{
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	int l;
	if (!enabled) return 0;
	if (ftw != w)
	{
	    int x;
		for (x = 0; x < 512; x ++)
		{
			int yp;
			int xp;
			xp=(x%8)-4;
			yp=(x/8)%8-4;
			if (xp<0) xp++;
			if (yp<0) yp++;
			fudgetable[x]=w*yp+xp;
		}
		ftw=w;
	}
	if (isBeat&0x80000000) return 0;

	l=w*4;
	while (l-- > 0) *fbout++=*framebuffer++;
	l=w*(h-8);
	while (l-- > 0)
	{
		*fbout++ = framebuffer[fudgetable[rand()&511]];
		framebuffer++;
	}
	l=w*4;
	while (l-- > 0) *fbout++=*framebuffer++;
	return 1;
}

C_RBASE *R_Scat(char *desc)
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
			if (g_this->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
			return 1;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_CHECK1)
			{
				if (IsDlgButtonChecked(hwndDlg,IDC_CHECK1))
					g_this->enabled=1;
				else
					g_this->enabled=0;
			}
			return 0;
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_SCAT,hwndParent,g_DlgProc);
}
#else
C_RBASE *R_Scat(char *desc)
{return NULL; }
#endif