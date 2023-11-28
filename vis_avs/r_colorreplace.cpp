// alphachannel safe 11/21/99
#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "timing.h"
#include "../winamp/api_language.h"

#define C_THISCLASS C_ContrastEnhanceClass
#define MOD_NAME "Trans / Color Clip"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_TRANS_COLOR_CLIP,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    int enabled;
    int color_clip;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	if (len-pos >= 4) { enabled=GET_INT(); pos+=4; }
	if (len-pos >= 4) { color_clip=GET_INT(); pos+=4; }
}
int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	PUT_INT(enabled); pos+=4;
  PUT_INT(color_clip); pos+=4;
	return pos;
}




C_THISCLASS::C_THISCLASS()
{
  enabled=1;
  color_clip=RGB(32,32,32);
}

C_THISCLASS::~C_THISCLASS()
{
}
	
int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	unsigned int *f = (unsigned int *) framebuffer;
  int fs_r,fs_g,fs_b;
  int x=w*h;
  if (!enabled) return 0;

  fs_b=(color_clip>>16)&255;
  fs_g=(color_clip>>8)&255;
  fs_r=(color_clip)&255;

  while (x--)
  {
    int r=f[0]&255;
    int g=(f[0]>>8)&255;
    int b=(f[0]>>16)&255;
    int a=(f[0]&0xff000000);
    if (r <= fs_r && g <= fs_g && b <= fs_b)
      f[0]=a|fs_r|(fs_g<<8)|(fs_b<<16);
    f++;
  }
  return 0;
}

C_RBASE *R_ContrastEnhance(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DRAWITEM:
			{
				DRAWITEMSTRUCT *di=(DRAWITEMSTRUCT *)lParam;
				switch (di->CtlID)
				{
					case IDC_LC:
						GR_DrawColoredButton(di,g_this->color_clip);
					break;
				}
			}
		return 0;
		case WM_INITDIALOG:
      if (g_this->enabled) CheckDlgButton(hwndDlg,IDC_CHECK1,BST_CHECKED);
			return 1;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_CHECK1:
          g_this->enabled=IsDlgButtonChecked(hwndDlg,IDC_CHECK1)?1:0;
          return 0;
				case IDC_LC:
					GR_SelectColor(hwndDlg,&g_this->color_clip);
					InvalidateRect(GetDlgItem(hwndDlg,LOWORD(wParam)),NULL,FALSE);
				return 0;

      }
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_CONTRASTENHANCE,hwndParent,g_DlgProc);
}