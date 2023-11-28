#include <windows.h>
#include <commctrl.h>
#include "r_defs.h"
#include "resource.h"
#include "../Agave/Language/api_language.h"

#define MOD_NAME "Unknown Render Object"

#include "r_unkn.h"

char *C_UnknClass::get_desc() { static char desc[128]; return (!desc[0]?WASABI_API_LNGSTRING_BUF(IDS_UNKNOWN_RENDER_OBJECT,desc,128):desc); }
void C_UnknClass::SetID(int d, char *dString) { id=d; memset(idString,0,sizeof(idString)); strcpy(idString,dString); }

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255
#define GET_INT() (data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24))
void C_UnknClass::load_config(unsigned char *data, int len)
{
	if (configdata) GlobalFree(configdata);
	configdata=(char *)GlobalAlloc(GMEM_FIXED,len);
	memcpy(configdata,data,len);
	configdata_len=len;
}
int  C_UnknClass::save_config(unsigned char *data)
{
	int pos=0;
	memcpy(data+pos,configdata,configdata_len);
	pos+=configdata_len;
	return pos;
}

C_UnknClass::C_UnknClass()
{
	configdata=0;
	configdata_len=0;
	id=0;
	idString[0]=0;
}

C_UnknClass::~C_UnknClass()
{
	if (configdata) GlobalFree(configdata);
	configdata=0;
}
	
int C_UnknClass::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h)
{
	return 0;
}

static C_UnknClass *g_this;

BOOL CALLBACK C_UnknClass::g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				char s[512]={0};
				if (g_this->idString[0]) wsprintf(s,"APE: %s\r\n",g_this->idString);
				else wsprintf(s,WASABI_API_LNGSTRING(IDS_BUILTIN_ID),g_this->id);
				wsprintf(s+strlen(s),WASABI_API_LNGSTRING(IDS_CONFIG_SIZE),g_this->configdata_len);
				SetDlgItemText(hwndDlg,IDC_EDIT1,s);
			}
 			return 1;
	}
	return 0;
}

HWND C_UnknClass::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_UNKN,hwndParent,g_DlgProc);
}