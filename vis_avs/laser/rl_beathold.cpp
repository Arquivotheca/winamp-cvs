#ifdef LASER
#include <windows.h>
#include <commctrl.h>
#include "../r_defs.h"
#include "../resource.h"
#include <math.h>

#define C_THISCLASS CLASER_BeatHoldClass
#define MOD_NAME "Misc / Beat Hold"

class C_THISCLASS : public C_RBASE {
	protected:
	public:
		C_THISCLASS();
		virtual ~C_THISCLASS();
		virtual int render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h); // returns 1 if fbout has dest
		virtual char *get_desc() { return MOD_NAME; }
		virtual HWND conf(HINSTANCE hInstance, HWND hwndParent);
		virtual void load_config(unsigned char *data, int len);
		virtual int  save_config(unsigned char *data);

    int decayMS, beatSkip;

    DWORD isBeatDecay;
    int beatCount;

    LineType m_linelist[1024];
    int linelist_used;
};

#define PUT_INT(y) data[pos]=(y)&255; data[pos+1]=(y>>8)&255; data[pos+2]=(y>>16)&255; data[pos+3]=(y>>24)&255; pos+=4
#define GET_INT(a) if (len-pos>=4) { (a)=(data[pos]|(data[pos+1]<<8)|(data[pos+2]<<16)|(data[pos+3]<<24)); pos += 4; }
void C_THISCLASS::load_config(unsigned char *data, int len)
{
	int pos=0;
  int x=0;
  GET_INT(decayMS);
  GET_INT(beatSkip);
}

int  C_THISCLASS::save_config(unsigned char *data)
{
	int pos=0;
  PUT_INT(decayMS);
  PUT_INT(beatSkip);
	return pos;
}


C_THISCLASS::C_THISCLASS()
{
  isBeatDecay=0;
  decayMS=500;
  beatCount=0;
  beatSkip=4;
  linelist_used=0;
}

C_THISCLASS::~C_THISCLASS()
{
}


int C_THISCLASS::render(char visdata[2][2][576], int isBeat, int *framebuffer, int *fbout, int w, int h) // returns 1 if fbout has dest
{
  if (isBeat&0x80000000) return 0;
  if (isBeatDecay) 
  {
    if (GetTickCount() > isBeatDecay)
    {
      isBeatDecay=0;
    }
    else
    {
      g_laser_linelist->SetLines(m_linelist,0,linelist_used);
      g_laser_linelist->SetUsedLines(linelist_used);
    }
  }
  else if (isBeat && ++beatCount > beatSkip)
  {
    beatCount=0;
    isBeatDecay=GetTickCount()+decayMS;
    linelist_used=g_laser_linelist->GetUsedLines();
    memcpy(m_linelist,g_laser_linelist->GetLineList(),linelist_used*sizeof(LineType));
  }
  return 0;
}

C_RBASE *RLASER_BeatHold(char *desc)
{
	if (desc) { strcpy(desc,MOD_NAME); return NULL; }
	return (C_RBASE *) new C_THISCLASS();
}


static C_THISCLASS *g_this;

static BOOL CALLBACK g_DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	int *a=NULL;
	switch (uMsg)
	{
		case WM_INITDIALOG:      
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMIN,0,1);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETRANGEMAX,0,20);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER1,TBM_SETPOS,1,g_this->decayMS/100);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMIN,0,0);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETRANGEMAX,0,16);
			SendDlgItemMessage(hwndDlg,IDC_SLIDER2,TBM_SETPOS,1,g_this->beatSkip);

    return 1;
		case WM_HSCROLL:
			{
				HWND swnd = (HWND) lParam;
				int t = (int) SendMessage(swnd,TBM_GETPOS,0,0);
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER1))
				{
					g_this->decayMS=t*100;
        }
				if (swnd == GetDlgItem(hwndDlg,IDC_SLIDER2))
				{
					g_this->beatSkip=t;
				}
			}
    return 0;
	}
	return 0;
}


HWND C_THISCLASS::conf(HINSTANCE hInstance, HWND hwndParent)
{
	g_this = this;
	return WASABI_API_CREATEDIALOG(IDD_CFG_LASER_BEATHOLD,hwndParent,g_DlgProc);
}
#endif