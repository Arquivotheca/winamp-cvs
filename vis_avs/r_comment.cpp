// alphachannel safe 11/21/99
#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "timing.h"
#include "../Agave/Language/api_language.h"

#define C_THISCLASS C_CommentClass
#define MOD_NAME "Misc / Comment"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h);
		virtual char *get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_MISC_COMMENT,desc,128):desc); }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

		RString msgdata;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
	load_string(msgdata,data,pos,len);
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
	save_string(data,pos,msgdata);
	return pos;
}

C_THISCLASS::C_THISCLASS()
{
	msgdata.assign("");
}

C_THISCLASS::~C_THISCLASS()
{
}

int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	return 0;
}

C_RBASE *R_Comment(char *desc)
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
			SetDlgItemText(hwndDlg,IDC_EDIT1,g_this->msgdata.get());
			return 1;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_EDIT1 && HIWORD(wParam) == EN_CHANGE)
			{
				g_this->msgdata.get_from_dlgitem(hwndDlg,IDC_EDIT1);
			}
			return 0;
	}
	return 0;
}

HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_COMMENT,hwndParent,g_DlgProc);
}