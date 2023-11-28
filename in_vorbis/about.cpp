#include "main.h"
#include "api.h"
#include "resource.h"
#include <strsafe.h>

static UINT xiphframes_ids[12]={IDB_BITMAP1,IDB_BITMAP2,IDB_BITMAP3,IDB_BITMAP4,IDB_BITMAP5,IDB_BITMAP6,IDB_BITMAP7,IDB_BITMAP8,IDB_BITMAP9,IDB_BITMAP10,IDB_BITMAP11,IDB_BITMAP12};
static HBITMAP xiphframes[12];

static void slap(HWND wnd,int v)
{
	long hi=GetWindowLong(wnd,4);
	if (v) hi+=v*1000;
	else hi=0;
	SetWindowLong(wnd,4,hi);
}

static CfgInt cfg_rpm("rpm",0);

static int visible_rpm,visible_max_rpm;
static bool tilt;
static bool showtilt;
static char show_rpm=0;
static DWORD last_visible_rpm;

static LRESULT WINAPI XiphProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_CREATE:
		SetWindowLong(wnd,8,last_visible_rpm=GetTickCount());
		SetTimer(wnd,666,10,0);
		visible_rpm=-1;
		visible_max_rpm=-1;
		show_rpm=0;
		tilt=0;
		break;
	case WM_TIMER:
		if (wp==666)
		{
			long low=GetWindowLong(wnd,0);
			long hi=GetWindowLong(wnd,4);

			long org=low&~0xFFFF;

			int rpm=MulDiv(abs(hi),1000*60,12*0x10000);

			DWORD t=GetTickCount();
			DWORD ot=(DWORD)SetWindowLong(wnd,8,t);
			bool redraw=0;

			if (rpm>150) show_rpm=1;
			if (cfg_rpm<rpm) cfg_rpm=rpm;

			if (!rpm)
			{
				tilt=0;
				if (showtilt) {showtilt=0;redraw=1;}
			}
			if (tilt)
			{
				bool st=(t&0x100)==0;
				if (st!=showtilt)
				{
					showtilt=st;
					redraw=1;
				}
			}
			else if (rpm>=2000)
			{
				tilt=1;
			}
			if (show_rpm && (t&~0x3F)!=(ot&~0x3F))
			{
				wchar_t foo[128];
				if (visible_rpm<rpm || (visible_rpm>rpm && (t-last_visible_rpm)>333))
				{
					last_visible_rpm=t;
					visible_rpm=rpm;
					StringCchPrintfW(foo,128,WASABI_API_LNGSTRINGW(IDS_GAME_SPEED),rpm);
					SetDlgItemTextW(GetParent(wnd),IDC_RPM,foo);
				}
				if (visible_max_rpm!=cfg_rpm)
				{
					visible_max_rpm=cfg_rpm;
					StringCchPrintfW(foo,128,WASABI_API_LNGSTRINGW(IDS_BEST_RPM),(int)cfg_rpm);
					SetDlgItemTextW(GetParent(wnd),IDC_RPM2,foo);
				}
			}

			low+=hi*(t-ot);
			while(low<0) low+=12*0x10000;
			while(low>=12*0x10000) low-=12*0x10000;

			{
				int z=hi>>6;
				if (z) hi-=z;
				else if (hi>0) hi--;
				else if (hi<0) hi++;
			}

			SetWindowLong(wnd,0,low);
			SetWindowLong(wnd,4,hi);
			if (redraw || (low&~0xFFFF)!=org)
			{
				RedrawWindow(wnd,0,0,RDW_INVALIDATE);
			}
			KillTimer(wnd,666);
			SetTimer(wnd,666,10,0);
		}
		break;
	case WM_LBUTTONDOWN:
		slap(wnd,-1);
		break;
	case WM_RBUTTONDOWN:
		slap(wnd,1);
		break;
	case WM_MBUTTONDOWN:
		slap(wnd,0);
		break;
	case WM_PAINT:
		{
			int i=(GetWindowLong(wnd,0))>>16;
			HBITMAP hBmp;
			if (!xiphframes[i])
			{
				xiphframes[i]=LoadBitmap(mod.hDllInstance,(char*)xiphframes_ids[i]);
			}
			hBmp=xiphframes[i];
			
			if (hBmp)
			{
				HDC dc=CreateCompatibleDC(0);
				HGDIOBJ foo=SelectObject(dc,hBmp);
				HDC wdc=GetDC(wnd);
				RECT r;
				GetClientRect(wnd,&r);
				DrawEdge(wdc,&r,BDR_SUNKENINNER|BDR_SUNKENOUTER,BF_LEFT|BF_TOP|BF_RIGHT|BF_BOTTOM);//BF_ADJUST
				BitBlt(wdc,2,2,r.right-2,r.bottom-2,dc,0,0,SRCCOPY);
				if (showtilt)
				{
					SetTextColor(wdc,0xFF);
					SetBkMode(wdc,TRANSPARENT);
					SetTextAlign(wdc,TA_CENTER|VTA_CENTER);
					TextOut(wdc,r.right>>1,r.bottom>>1,"TILT",4);
				}				
				ReleaseDC(wnd,wdc);
				SelectObject(dc,foo);
				DeleteDC(dc);
			}
		}
		break;
	case WM_DESTROY:
		KillTimer(wnd,666);
		break;
	};
	return DefWindowProc(wnd,msg,wp,lp);
}

static BOOL CALLBACK AboutProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
	{
		wchar_t tmp[1024] = {0}, tmp2[1024] = {0}, *t1 = tmp, *t2 = tmp2, text[1024];
		SetWindowTextW(wnd,WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_VORBIS_DECODER_OLD,text,1024));
		int len = StringCchPrintfW(tmp,1024,WASABI_API_LNGSTRINGW(IDS_ABOUT_TEXT),mod.description,__DATE__);
		// due to quirks with the more common resource editors, is easier to just store the string
		// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
		// on new lines as is wanted (silly multiline edit controls)
		while(t1 && *t1 && (t2 - tmp2 < 1024))
		{
			if(*t1 == L'\n')
			{
				*t2 = L'\r';
				t2 = CharNextW(t2);
			}
			*t2 = *t1;
			t1 = CharNextW(t1);
			t2 = CharNextW(t2);
		}

		SetDlgItemTextW(wnd,IDC_ABOUT_TEXT,tmp2);
		// fixes the incorrect selection of the text on dialog opening
		PostMessage(GetDlgItem(wnd,IDC_ABOUT_TEXT),EM_SETSEL,-1,0);
		return 1;
	}
	case WM_COMMAND:
		if (wp==IDOK || wp==IDCANCEL) EndDialog(wnd,0);
		break;
	}
	return 0;
}

void About(HWND hwndParent)
{
	static char got_xiph;
	if (!got_xiph)
	{
		WNDCLASS wc=
		{
			0,
			XiphProc,
			0,
			12,
			WASABI_API_LNG_HINST,
			0,
			LoadCursor(0,IDC_ARROW),
			0,
			0,
			"XIPH_CLASS",
		};

		RegisterClass(&wc);
		got_xiph=1;
	}

	WASABI_API_DIALOGBOXW(IDD_ABOUT,hwndParent,AboutProc);
}