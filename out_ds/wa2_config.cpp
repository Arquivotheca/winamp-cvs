#include "out_ds.h"
#include <windowsx.h>
#include <commctrl.h>
#include "res_wa2/resource.h"
#include "ds2.h"
#include <math.h>
#include <stdio.h>
#include "ds_ipc.h"
#include "api.h"
#include "../winamp/wa_ipc.h"

bool wa2_GetRealtimeStat(DS2_REALTIME_STAT * stat);

void wa2_sync_in();//hack, use critical section on config changes
void wa2_sync_out();

#ifdef HAVE_SSRC
bool wa2_IsResampling(RESAMPLING_STATUS *);
#endif

FadeCfg
cfg_fade_start("start","cfg_fade_start",333,0,1),
cfg_fade_firststart("first start","cfg_fade_firststart",333,0,1),
cfg_fade_stop("end of song","cfg_fade_stop",333,0,1),
cfg_fade_pause("pause/stop","cfg_fade_pause",333,1,1),
cfg_fade_seek("seek","cfg_fade_seek",333,1,1);

#define N_FADES 5
static FadeCfg * fades[N_FADES]={&cfg_fade_start,&cfg_fade_firststart,&cfg_fade_stop,&cfg_fade_pause,&cfg_fade_seek};
int FadeNames[N_FADES] = {IDS_FADE_START, IDS_FADE_FIRSTSTART, IDS_FADE_STOP, IDS_FADE_PAUSE, IDS_FADE_SEEK};

HWND buffer_config_wnd = NULL;
HWND fades_config_wnd = NULL;
extern HWND ipcWnd;
extern Out_Module mod;

cfg_int cfg_def_fade("cfg_def_fade",333);

static DWORD config_start_time;

extern cfg_int cfg_oldpause;
extern cfg_int cfg_volmode;
extern cfg_int cfg_logvol_min;
extern cfg_int cfg_logfades;

extern cfg_int cfg_fadevol;
extern cfg_int cfg_buf_ms,cfg_prebuf2,cfg_sil_db,cfg_trackhack;
extern cfg_struct_t<GUID> cfg_dev2;
extern cfg_int cfg_wait,cfg_killsil,cfg_volume;
extern cfg_int cfg_hw_mix;
extern cfg_int cfg_createprimary,cfg_override;
extern cfg_int cfg_override_freq,cfg_override_bps,cfg_override_nch;
extern cfg_struct_t<__int64> cfg_total_time;
extern cfg_int cfg_autocpu;
static cfg_int cfg_status_update_freq("cfg_status_update_freq",50);
extern cfg_int cfg_cur_tab;

static char * rstrcpy(char * s1,const char * s2)
{
	while(*s2) *(s1++)=*(s2++);
	return s1;
}

typedef struct
{
	const char * name;
	int on,usedef;
	int time;
} FadeCfgCopy;

void format_fade(char * txt,FadeCfgCopy * c, int idx)
{
	txt=rstrcpy(txt, WASABI_API_LNGSTRING(IDS_ON));
	txt=rstrcpy(txt, WASABI_API_LNGSTRING(FadeNames[idx]));
	if (!c->on) txt=rstrcpy(txt,WASABI_API_LNGSTRING(IDS_DISABLED));
	else if (!c->usedef)
	{
		char tmp[16], fmt[16];
		wsprintf(fmt," (%s)",WASABI_API_LNGSTRING(IDS_DS_U_MS));
		wsprintf(tmp,fmt,c->time);
		txt=rstrcpy(txt,tmp);
	}
	*txt=0;
}

static void add_fade(HWND w,FadeCfg * cfg, int n)
{
	FadeCfgCopy * c=new FadeCfgCopy;
	c->name=cfg->name;
	c->time=cfg->time;
	c->on=cfg->on;
	c->usedef=cfg->usedef;
	char txt[256];
	format_fade(txt,c,n);
	UINT i=SendMessage(w,LB_ADDSTRING,0,(LPARAM)txt);
	SendMessage(w,LB_SETITEMDATA,i,(LPARAM)c);
}

static void update_prebuf_1(HWND wnd)
{
	char zz[128];
	wsprintf(zz,WASABI_API_LNGSTRING(IDS_DS_U_MS),SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_GETPOS,0,0));
	SetDlgItemText(wnd,IDC_PREBUF_DISP_1,zz);
}

static void update_prebuf_2(HWND wnd)
{
	char zz[128];
	wsprintf(zz,WASABI_API_LNGSTRING(IDS_DS_U_MS),SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_GETPOS,0,0));
	SetDlgItemText(wnd,IDC_PREBUF_DISP_2,zz);
}

#define BUFFER_SCALE 4000.0

UINT cur_buffer;

static UINT get_buffer(HWND wnd)
{
	if (cur_buffer) return cur_buffer;

	//0-BUFFER_SCALE => 200-20000	
	int z=SendDlgItemMessage(wnd,IDC_BUFFER,TBM_GETPOS,0,0);
	return cur_buffer=(UINT) ( 0.5 + 200.0*pow(100.0,(double)z/BUFFER_SCALE) );
}

void set_buffer(HWND wnd,UINT b)
{
	cur_buffer=b;
	SendDlgItemMessage(wnd,IDC_BUFFER,TBM_SETPOS,1,(long) ( 0.5 + BUFFER_SCALE * log( (double)b/200.0 ) / log( 100.0 ) ));
}

void update_prebuf_range(HWND wnd)
{
	UINT max=get_buffer(wnd);
	if (max>0x7FFF) max=0x7FFF;
	SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETRANGE,1,MAKELONG(0,max));
	SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETRANGE,1,MAKELONG(0,max));
}


void update_buf(HWND wnd)
{
	char zz[128];
	wsprintf(zz,WASABI_API_LNGSTRING(IDS_DS_U_MS),get_buffer(wnd));
	SetDlgItemText(wnd,IDC_BUF_DISP,zz);
}


static void _switch_dlgitems(HWND wnd,const UINT * ids,UINT n_ids,int b)
{
	UINT n;
	for(n=0;n<n_ids;n++)
	{
		EnableWindow(GetDlgItem(wnd,ids[n]),b);
	}
}

#define switch_dlgitems(W,X,B) _switch_dlgitems(W,X,sizeof(X)/sizeof(X[0]),B)

#pragma warning(disable:4800)

static void cfgSetDevice(HWND wnd,const GUID * guid,const char * name)
{
	HWND w=GetDlgItem(wnd,IDC_DEVICE);
	SendMessage(w,CB_RESETCONTENT,0,0);
	DsDevEnum e;
	bool dev_set=0;
	UINT n=0;
	while(e)
	{
		SendMessage(w,CB_ADDSTRING,0,(LPARAM)e.GetName());
		if (!dev_set)
		{
			if (name)
			{
				if (!lstrcmpi(e.GetName(),name)) dev_set=1;
			}
			else
			{
				if (e.GetGuid()==*guid) dev_set=1;
			}

			if (dev_set) SendMessage(w,CB_SETCURSEL,n,0);
		}
		n++;
		e++;
	}
	if (!dev_set) SendMessage(w,CB_SETCURSEL,0,0);
	SendMessage(wnd,WM_COMMAND,(CBN_SELCHANGE<<16)|IDC_DEVICE,0);
}

static INT_PTR CALLBACK CfgProc1(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//device
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(wnd,IDC_HW_MIX,BM_SETCHECK,cfg_hw_mix,0);
		SendDlgItemMessage(wnd,IDC_CREATE_PRIMARY,BM_SETCHECK,cfg_createprimary,0);
		cfgSetDevice(wnd,&(GUID)cfg_dev2,0);
		{
			DsDevEnum dev_enum;
			if (dev_enum.FindDefault())
			{
				char blah[512];
				wsprintf(blah,WASABI_API_LNGSTRING(IDS_FAQ_PREFERRED_DEVICE),dev_enum.GetName());
				SetDlgItemText(wnd,IDC_PDS_FAQ,blah);
			}
			else
			{
				SetDlgItemText(wnd,IDC_PDS_FAQ,WASABI_API_LNGSTRING((IDC_PDS_FAQ,DS2::InitDLL() ? IDS_NO_DS_DEVICES_PRESENT : IDS_DS_DOES_NOT_APPEAR_TO_BE_INSTALLED)));
				ShowWindow(GetDlgItem(wnd,IDC_STATIC_BLEH),SW_HIDE);
			}
		}
		
		return 1;
	case WM_COMMAND:
		switch(LOWORD(wp))
		{
		case IDC_REFRESH:
			{
				char name[256];
				GetDlgItemText(wnd,IDC_DEVICE,name,256);
				cfgSetDevice(wnd,0,name);
			}

			break;
		case IDC_DEVICE:
			if(HIWORD(wp) == CBN_SELCHANGE)
			{
				DSCAPS caps;
				DWORD speakercfg;
				char name[256];
				GetDlgItemText(wnd,IDC_DEVICE,name,256);

				DsDevEnum dev_enum;
				
				EnableWindow(GetDlgItem(wnd,IDC_HW_MIX),1);
				
				if (!dev_enum)
				{
					SetDlgItemText(wnd,IDC_DEVICE_INFO,WASABI_API_LNGSTRING(IDS_NO_DEVICES_FOUND));
				}
				else if (!dev_enum.FindName(name))
				{
					SetDlgItemText(wnd,IDC_DEVICE_INFO,WASABI_API_LNGSTRING(IDS_DEVICE_NOT_FOUND));
				}
				else if (!dev_enum.GetCaps(&caps,&speakercfg))
				{
					SetDlgItemText(wnd,IDC_DEVICE_INFO,WASABI_API_LNGSTRING(IDS_ERROR_GETTING_DEVICE_INFO));
				}
				else
				{
					char mixblah[256];
					bool canmix=caps.dwMaxHwMixingStreamingBuffers>1;
					if (!canmix) WASABI_API_LNGSTRING_BUF(IDS_UNSUPPORTED,mixblah,256);
					else wsprintf(mixblah,WASABI_API_LNGSTRING(IDS_SUPPORTED_X_FREE_STREAMS),caps.dwFreeHwMixingStreamingBuffers,caps.dwMaxHwMixingStreamingBuffers);
					char memblah[256];
					if (caps.dwTotalHwMemBytes>0) wsprintf(memblah,WASABI_API_LNGSTRING(IDS_X_BYTES),caps.dwTotalHwMemBytes,caps.dwFreeHwMemBytes);
					else WASABI_API_LNGSTRING_BUF(IDS_NA,memblah,256);

					char spkcfg[64];
					const char *p_speakercfg = spkcfg;
					switch(DSSPEAKER_CONFIG(speakercfg))
					{
						case DSSPEAKER_5POINT1:
							WASABI_API_LNGSTRING_BUF(IDS_5_1,spkcfg,64);
							break;
						case DSSPEAKER_HEADPHONE:
							WASABI_API_LNGSTRING_BUF(IDS_HEADPHONES,spkcfg,64);
							break;
						case DSSPEAKER_MONO:
							WASABI_API_LNGSTRING_BUF(IDS_MONO,spkcfg,64);
							break;
						case DSSPEAKER_QUAD:
							WASABI_API_LNGSTRING_BUF(IDS_QUAD,spkcfg,64);
							break;
						case DSSPEAKER_STEREO:
							WASABI_API_LNGSTRING_BUF(IDS_STEREO,spkcfg,64);
							break;
						case DSSPEAKER_SURROUND:
							WASABI_API_LNGSTRING_BUF(IDS_SURROUND,spkcfg,64);
							break;
						case DSSPEAKER_7POINT1_SURROUND:
							WASABI_API_LNGSTRING_BUF(IDS_7_1,spkcfg,64);
							break;
						default:
							WASABI_API_LNGSTRING_BUF(IDS_UNKNOWN,spkcfg,64);
							break;
					}

					char blah[1024], t1[16], t2[16], t3[32];
					wsprintf(blah,
							 WASABI_API_LNGSTRING(IDS_DS_INFO),
							 WASABI_API_LNGSTRING_BUF((caps.dwFlags&DSCAPS_CERTIFIED?IDS_YES:IDS_NO),t1,8),
							 WASABI_API_LNGSTRING_BUF((caps.dwFlags&DSCAPS_EMULDRIVER?IDS_YES:IDS_NO),t2,8),
							 caps.dwMinSecondarySampleRate,caps.dwMaxSecondarySampleRate,
							 (caps.dwFlags&DSCAPS_CONTINUOUSRATE) ? WASABI_API_LNGSTRING_BUF(IDS_CONTINUOUS,t3,16) : "",
							 memblah,
							 mixblah,
							 p_speakercfg);

					SetDlgItemText(wnd,IDC_DEVICE_INFO,blah);
				}
			}
			break;
		case IDC_APPLY:
		case IDOK:
			{
				char name[256];
				GetDlgItemText(wnd,IDC_DEVICE,name,256);
				cfg_dev2=DsDevEnumName(name).GetGuid();
			}
			cfg_hw_mix=(bool)SendDlgItemMessage(wnd,IDC_HW_MIX,BM_GETCHECK,0,0);
			cfg_createprimary=(bool)SendDlgItemMessage(wnd,IDC_CREATE_PRIMARY,BM_GETCHECK,0,0);
			break;
		case IDCANCEL:
			break;
		}
		break;
	}
	return 0;
}

static const UINT prebuf_ctrls[]={IDC_PREBUFFER_1,IDC_PREBUF_DISP_1};

static INT_PTR CALLBACK CfgProc2(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//buffering
	switch(msg)
	{
	case WM_INITDIALOG:
		buffer_config_wnd = wnd;
		SendDlgItemMessage(wnd,IDC_BUFFER,TBM_SETRANGE,0,MAKELONG(0,(int)BUFFER_SCALE));
		set_buffer(wnd,cfg_buf_ms);
		update_prebuf_range(wnd);
		SendDlgItemMessage(wnd,IDC_PREBUF_AUTO,BM_SETCHECK,cfg_autocpu,0);
		SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETPOS,1,cfg_prebuf2);
		SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETPOS,1,cfg_trackhack);
		update_prebuf_1(wnd);
		update_prebuf_2(wnd);
		update_buf(wnd);
		return 1;
	case WM_DESTROY:
		buffer_config_wnd = NULL;
		return 0;
	case WM_HSCROLL:
		switch(GetWindowLong((HWND)lp,GWL_ID))
		{
		case IDC_BUFFER:
			cur_buffer=0;
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
	case WM_COMMAND:
		switch(wp)
		{
		case IDC_BUF_RESET:
			set_buffer(wnd,cfg_buf_ms.get_def());
			update_prebuf_range(wnd);
			SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_SETPOS,1,cfg_prebuf2.get_def());
			SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_SETPOS,1,cfg_trackhack.get_def());
			update_prebuf_1(wnd);
			update_prebuf_2(wnd);
			update_buf(wnd);
			break;
		case IDC_APPLY:
		case IDOK:
			cfg_buf_ms=get_buffer(wnd);
			cfg_prebuf2=SendDlgItemMessage(wnd,IDC_PREBUFFER_1,TBM_GETPOS,0,0);
			cfg_trackhack=SendDlgItemMessage(wnd,IDC_PREBUFFER_2,TBM_GETPOS,0,0);
			cfg_autocpu=SendDlgItemMessage(wnd,IDC_PREBUF_AUTO,BM_GETCHECK,0,0);
			break;
		case IDCANCEL:

			break;
		}
		break;
	}

	const int controls[] = 
	{
		IDC_DB,
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

static const UINT customfade_ids[]={IDC_FADE_GROUP,IDC_USE_CUSTOM_FADE,IDC_CUSTOM_FADE,IDC_CUSTOM_FADE_SPIN,IDC_STATIC_MS,IDC_FADE_ENABLED};

static INT_PTR CALLBACK CfgProc3(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//fading
	switch(msg)
	{
	case WM_INITDIALOG:
		fades_config_wnd = wnd;
		SendDlgItemMessage(wnd,IDC_CUSTOM_FADE_SPIN,UDM_SETRANGE,0,MAKELONG(20000,0));
		SendDlgItemMessage(wnd,IDC_FADE_SPIN,UDM_SETRANGE,0,MAKELONG(0x7FFF,0));
		SetDlgItemInt(wnd,IDC_FADE,cfg_def_fade,0);
		SendDlgItemMessage(wnd,IDC_PAUSEFADE2,BM_SETCHECK,cfg_oldpause,0);

		{
			HWND w;
			UINT n;
			w=GetDlgItem(wnd,IDC_LIST);
			for(n=0;n<N_FADES;n++)
			{
				add_fade(w,fades[n],n);
			}
		}

		SendDlgItemMessage(wnd,IDC_WAITx,BM_SETCHECK,cfg_wait,0);

		if (NULL != WASABI_API_APP)
			WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(GetDlgItem(wnd, IDC_LIST), TRUE);

		return 1;
	case WM_DESTROY:
		fades_config_wnd = NULL;
		if (NULL != WASABI_API_APP)
			WASABI_API_APP->DirectMouseWheel_EnableConvertToMouseWheel(GetDlgItem(wnd, IDC_LIST), FALSE);
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case (LBN_DBLCLK<<16)|IDC_LIST:
			{
				int idx=SendMessage((HWND)lp,LB_GETCURSEL,0,0);
				if (idx!=-1)
				{
					FadeCfgCopy * c=(FadeCfgCopy*)SendMessage((HWND)lp,LB_GETITEMDATA,idx,0);
					c->on=!c->on;
					char txt[256];
					format_fade(txt,c,idx);
					SendMessage((HWND)lp,LB_DELETESTRING,idx,0);
					SendMessage((HWND)lp,LB_INSERTSTRING,idx,(LPARAM)txt);
					SendMessage((HWND)lp,LB_SETITEMDATA,idx,(LPARAM)c);
					SendMessage((HWND)lp,LB_SETCURSEL,idx,0);
					SendDlgItemMessage(wnd,IDC_FADE_ENABLED,BM_SETCHECK,c->on,0);
				}
			}
			break;
		case (LBN_SELCHANGE<<16)|IDC_LIST:
			{
				int idx=SendMessage((HWND)lp,LB_GETCURSEL,0,0);
				if (idx!=-1)
				{
					FadeCfgCopy * c=(FadeCfgCopy*)SendMessage((HWND)lp,LB_GETITEMDATA,idx,0);
					SendDlgItemMessage(wnd,IDC_USE_CUSTOM_FADE,BM_SETCHECK,!c->usedef,0);
					SetDlgItemInt(wnd,IDC_CUSTOM_FADE,c->time,0);
					char boo[128], tmp[32];
					wsprintf(boo,WASABI_API_LNGSTRING(IDS_FADE_ON_X_SETTINGS),WASABI_API_LNGSTRING_BUF(FadeNames[idx],tmp,32));
					SetDlgItemText(wnd,IDC_FADE_GROUP,boo);
					SendDlgItemMessage(wnd,IDC_FADE_ENABLED,BM_SETCHECK,c->on,0);
					switch_dlgitems(wnd,customfade_ids,1);
				}
				else
				{
					SendDlgItemMessage(wnd,IDC_USE_CUSTOM_FADE,BM_SETCHECK,0,0);
					SendDlgItemMessage(wnd,IDC_FADE_ENABLED,BM_SETCHECK,0,0);
					SetDlgItemText(wnd,IDC_CUSTOM_FADE,"");
					SetDlgItemText(wnd,IDC_FADE_GROUP,"");
					switch_dlgitems(wnd,customfade_ids,0);
				}
			}
			break;
		case (EN_CHANGE<<16)|IDC_CUSTOM_FADE:
			{
				HWND list=GetDlgItem(wnd,IDC_LIST);
				int idx=SendMessage(list,LB_GETCURSEL,0,0);
				if (idx>=0)
				{
					FadeCfgCopy * c=(FadeCfgCopy*)SendMessage(list,LB_GETITEMDATA,idx,0);
					c->time=GetDlgItemInt(wnd,IDC_CUSTOM_FADE,0,0);
					if (!c->usedef)
					{
						char txt[256];
						format_fade(txt,c,idx);
						SendMessage(list,LB_DELETESTRING,idx,0);
						SendMessage(list,LB_INSERTSTRING,idx,(LPARAM)txt);
						SendMessage(list,LB_SETITEMDATA,idx,(LPARAM)c);
						SendMessage(list,LB_SETCURSEL,idx,0);
					}
				}
			}
			break;
		case IDC_USE_CUSTOM_FADE:
			{
				HWND list=GetDlgItem(wnd,IDC_LIST);
				int idx=SendMessage(list,LB_GETCURSEL,0,0);
				if (idx>=0)
				{
					FadeCfgCopy * c=(FadeCfgCopy*)SendMessage(list,LB_GETITEMDATA,idx,0);
					c->usedef=!SendMessage((HWND)lp,BM_GETCHECK,0,0);
					char txt[256];
					format_fade(txt,c,idx);
					SendMessage(list,LB_DELETESTRING,idx,0);
					SendMessage(list,LB_INSERTSTRING,idx,(LPARAM)txt);
					SendMessage(list,LB_SETITEMDATA,idx,(LPARAM)c);
					SendMessage(list,LB_SETCURSEL,idx,0);
				}
			}
			break;
		case IDC_FADE_ENABLED:
			{
				HWND list=GetDlgItem(wnd,IDC_LIST);
				int idx=SendMessage(list,LB_GETCURSEL,0,0);
				if (idx>=0)
				{
					FadeCfgCopy * c=(FadeCfgCopy*)SendMessage(list,LB_GETITEMDATA,idx,0);
					c->on=SendMessage((HWND)lp,BM_GETCHECK,0,0);
					char txt[256];
					format_fade(txt,c,idx);
					SendMessage(list,LB_DELETESTRING,idx,0);
					SendMessage(list,LB_INSERTSTRING,idx,(long)txt);
					SendMessage(list,LB_SETITEMDATA,idx,(long)c);
					SendMessage(list,LB_SETCURSEL,idx,0);
				}
			}
			break;
		case IDC_APPLY:
		case IDOK:
			{
				UINT n=GetDlgItemInt(wnd,IDC_FADE,0,0);
				/*if (n<0) n=0;
				else */if (n>50000) n=50000;
				cfg_def_fade=n;

				HWND w=GetDlgItem(wnd,IDC_LIST);
				for(n=0;n<N_FADES;n++)
				{
					FadeCfgCopy* c=(FadeCfgCopy*)SendMessage(w,LB_GETITEMDATA,n,0);
					fades[n]->time=c->time;
					fades[n]->on=c->on;
					fades[n]->usedef=c->usedef;
					if (wp==IDOK) delete c;
				}
			}
			cfg_oldpause=(bool)SendDlgItemMessage(wnd,IDC_PAUSEFADE2,BM_GETCHECK,0,0);
			cfg_wait=(bool)SendDlgItemMessage(wnd,IDC_WAITx,BM_GETCHECK,0,0);
      SendMessage(ipcWnd, WM_DS_IPC, 0, DS_IPC_CB_CFGREFRESH);
			break;
		case IDCANCEL:
			{
				HWND w=GetDlgItem(wnd,IDC_LIST);
				UINT n;
				for(n=0;n<N_FADES;n++)
				{
					delete (FadeCfgCopy*)SendMessage(w,LB_GETITEMDATA,n,0);
				}
				SendMessage(w,LB_RESETCONTENT,0,0);
			}
			break;
		}
		break;
	}
	return 0;
}

static const UINT logvol_ids[]={IDC_LOGVOL_STATIC,IDC_LOGVOL_MIN,IDC_LOGVOL_STATIC2,IDC_LOGVOL_SPIN};

static INT_PTR CALLBACK CfgProc4(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//other
	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(wnd,IDC_VOLUME,BM_SETCHECK,cfg_volume,0);

		{
			HWND w=GetDlgItem(wnd,IDC_VOLMODE);
			SendMessage(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRING(IDS_LINEAR));
			SendMessage(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRING(IDS_LOGARITHMIC));
			SendMessage(w,CB_ADDSTRING,0,(long)WASABI_API_LNGSTRING(IDS_HYBRID));
			SendMessage(w,CB_SETCURSEL,cfg_volmode,0);
		}

		SendDlgItemMessage(wnd,IDC_LOGVOL_SPIN,UDM_SETRANGE,0,MAKELONG(100,1));
		SetDlgItemInt(wnd,IDC_LOGVOL_MIN,cfg_logvol_min,0);
		if (cfg_volmode!=1) switch_dlgitems(wnd,logvol_ids,0);
		SendDlgItemMessage(wnd,IDC_LOGFADES,BM_SETCHECK,cfg_logfades,0);

		{
			HWND w;
			w=GetDlgItem(wnd,IDC_DB);
			SendMessage(w,TBM_SETRANGE,0,MAKELONG(150,2000));
			SendMessage(w,TBM_SETPOS,1,cfg_sil_db);
		}

		SendDlgItemMessage(wnd,IDC_KILLSIL,BM_SETCHECK,cfg_killsil,0);
		SendDlgItemMessage(wnd,IDC_FADEVOL,BM_SETCHECK,cfg_fadevol,0);
	
		CfgProc4(wnd,WM_HSCROLL,0,(long)GetDlgItem(wnd,IDC_DB));

		return 1;
	case WM_HSCROLL:
		switch(GetWindowLong((HWND)lp,GWL_ID))
		{
		case IDC_DB:
			{
				UINT foo=SendDlgItemMessage(wnd,IDC_DB,TBM_GETPOS,0,0);
				char zz[16];
				_snprintf(zz,16,"-%.1f %s", foo / 10.0f, WASABI_API_LNGSTRING(IDS_DS_DB));
				SetDlgItemText(wnd,IDC_DB_DISPLAY,zz);
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDC_VOLMODE|(CBN_SELCHANGE<<16):
			switch_dlgitems(wnd,logvol_ids,SendMessage((HWND)lp,CB_GETCURSEL,0,0)==1);
			break;
		case IDC_APPLY:
		case IDOK:
			cfg_volume=(bool)SendDlgItemMessage(wnd,IDC_VOLUME,BM_GETCHECK,0,0);
			cfg_fadevol=(bool)SendDlgItemMessage(wnd,IDC_FADEVOL,BM_GETCHECK,0,0);
			cfg_volmode=SendDlgItemMessage(wnd,IDC_VOLMODE,CB_GETCURSEL,0,0);
			cfg_logvol_min=GetDlgItemInt(wnd,IDC_LOGVOL_MIN,0,0);
			if (cfg_logvol_min<10) cfg_logvol_min=10;
			if (cfg_logvol_min>100) cfg_logvol_min=100;
			cfg_killsil=SendDlgItemMessage(wnd,IDC_KILLSIL,BM_GETCHECK,0,0);
			cfg_sil_db=SendDlgItemMessage(wnd,IDC_DB,TBM_GETPOS,0,0);
			cfg_logfades=SendDlgItemMessage(wnd,IDC_LOGFADES,BM_GETCHECK,0,0);
			break;
		case IDCANCEL:

			break;
		}
		break;
	}

	const int controls[] = 
	{
		IDC_DB,
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

static void FormatProgress(UINT pos,UINT max,UINT len,char * out)
{
	UINT pos1=MulDiv(pos,len,max);
	UINT n;
	*(out++)='[';
	for(n=0;n<len;n++)
	{
		*(out++)= (n==pos1) ? '#' : '=';
	}

	*(out++)=']';
	*(out++)=0;
}

static void FormatTime(__int64 t,char * out)
{
	int w, d, h, m, s;
	w = (int)(t / (1000 * 60 * 60 * 24 * 7));
	d = (int)(t / (1000 * 60 * 60 * 24)) % 7;
	h = (int)(t / (1000 * 60 * 60)) % 24;
	m = (int)(t / (1000 * 60)) % 60;
	s = (int)(t / (1000)) % 60;
	if (w)
	{
		_snprintf(out, 32, TEXT("%iw "), w);
		while (*out) out++;
	}
	if (d)
	{
		_snprintf(out, 32, TEXT("%id "), d);
		while (*out) out++;
	}
	if (h)
	{
		_snprintf(out, 32, TEXT("%i:"), h);
		while (*out) out++;
	}
	_snprintf(out, 32,h ? TEXT("%02i:") : TEXT("%i:"), m);
	while (*out) out++;
	_snprintf(out, 32, TEXT("%02i.%03d"), s, (t % 1000));
}

static INT_PTR CALLBACK CfgProc_stat(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	if(msg == WM_ERASEBKGND)
	{
		return 1;
	}
	return 0;
}

static INT_PTR CALLBACK CfgProc6(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//status
	static char display[1024];
	static char disp_devname[256];
	static GUID devguid;
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			HWND w=WASABI_API_CREATEDIALOGPARAMW(IDD_CONFIG_STATUS,wnd,CfgProc_stat,0);
			SendMessage(mod.hMainWindow,WM_WA_IPC,(WPARAM)w,IPC_USE_UXTHEME_FUNC);
			SetWindowLong(w, GWL_ID, IDC_STATUS);
		}
		disp_devname[0]=0;
		SendDlgItemMessage(wnd,IDC_REFRESH_SPIN,UDM_SETRANGE,0,MAKELONG(10000,10));
		SetDlgItemInt(wnd,IDC_REFRESH,cfg_status_update_freq,0);
		SetTimer(wnd,1,cfg_status_update_freq,0);
		
	case WM_TIMER:
		{
			char total[32];
			__int64 time_total=DS2::GetTotalTime();
			FormatTime(time_total,total);
			DS2_REALTIME_STAT stat;
			if (wa2_GetRealtimeStat(&stat))
			{
				char bigint1[32],bigint2[32];
				_i64toa_s(stat.bytes_written,bigint1,32,10);
				_i64toa_s(stat.bytes_played,bigint2,32,10);
				char time1[32],time2[32];
				FormatTime(stat.bytes_written/(stat.bps/8*stat.nch)*1000/stat.sr,time1);
				__int64 time_played=stat.bytes_played/(stat.bps/8*stat.nch)*1000/stat.sr;
				FormatTime(time_played,time2);
				
				char prog1[72],prog2[72];
				FormatProgress(stat.pos_play,stat.buf_size_bytes,50,prog1);
				FormatProgress(stat.pos_write,stat.buf_size_bytes,50,prog2);
#ifdef HAVE_SSRC
				char res_status[128];
				RESAMPLING_STATUS resfoo;
				if (!wa2_IsResampling(&resfoo)) lstrcpy(res_status,"inactive");
				else wsprintf(res_status,"active, %u Hz / %u bps => %u Hz / %u bps",resfoo.src_freq,resfoo.src_bps,resfoo.dst_freq,resfoo.dst_bps);
#endif
				if (!disp_devname[0] || devguid!=stat.current_device)
				{
					lstrcpy(disp_devname,DsDevEnumGuid(stat.current_device).GetName());
					devguid=stat.current_device;
				}

				// note:
				// with the HAVE_SSRC stuff, a new formatting string would be needed to be added to
				// the dll resources otherwise it gets complicated with string management of things
				char s1[16], s2[16], s3[16], s4[16], s5[16];
				sprintf_s(display,1024,WASABI_API_LNGSTRING(IDS_STATUS_TEXT)
/*#ifdef HAVE_SSRC
					 EOL "Resampling: %s"
#endif*/
					,
					stat.sr,
					stat.bps,
					stat.nch,
					WASABI_API_LNGSTRING_BUF(stat.nch>1 ? IDS_CHANNELS : IDS_CHANNEL,s1,16),
					stat.buf_size_ms,stat.buf_size_bytes,
					disp_devname,
					WASABI_API_LNGSTRING_BUF(((stat.dscaps_flags&DSBCAPS_LOCHARDWARE) ? IDS_HARDWARE : (stat.dscaps_flags&DSBCAPS_LOCSOFTWARE) ? IDS_SOFTWARE : IDS_UNKNOWN),s2,16),
					WASABI_API_LNGSTRING_BUF((stat.have_primary_buffer ? IDS_ACTIVE : IDS_INACTIVE),s3,16),
					WASABI_API_LNGSTRING_BUF(((stat.dscaps_flags_primary&DSBCAPS_LOCHARDWARE) ? IDS_HARDWARE_BRACKETED : (stat.dscaps_flags_primary&DSBCAPS_LOCSOFTWARE) ? IDS_SOFTWARE_BRACKETED : IDS_EMPTY),s4,16),
					stat.pos_play,
					WASABI_API_LNGSTRING_BUF((stat.paused?IDS_PAUSED_BRACKETED:IDS_EMPTY),s5,16),
					prog1,
					stat.pos_write,
					prog2,
					stat.latency_ms,
					stat.latency,
					MulDiv(stat.bytes_async,1000,stat.sr*stat.nch*(stat.bps>>3)),
					stat.bytes_async,
					stat.lock_count,
					stat.underruns,
					time2,bigint2,
					time1,bigint1,
					total,
					stat.vol_left,
					stat.vol_right
/*#ifdef HAVE_SSRC
					,res_status
#endif*/
				);
			}
			else wsprintf(display,WASABI_API_LNGSTRING(IDS_NOT_ACTIVE_TOTAL_PLAYED),total);

			HWND s = GetDlgItem(wnd, IDC_STATUS);
			SetWindowRedraw(s, 0);
			SetDlgItemText(s, IDC_STATUS, display);
			SetWindowRedraw(s, 1);
			InvalidateRect(s, 0, 0);
		}
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDOK:
		case IDCANCEL:
			KillTimer(wnd,1);
			break;
		case IDC_STAT_COPY:
			{
				if (OpenClipboard(wnd))
				{
					// due to quirks with the more common resource editors, is easier to just store the string
					// internally only with \n and post-process to be \r\n (as here) so it will appear correctly
					// on new lines as is wanted (silly multiline edit controls) when copying to the clipboard
					int len = 0;
					static char tmp2[1024] = {0};
					char *t1 = display, *t2 = tmp2;
					while(t1 && *t1 && (t2 - tmp2 < 1024))
					{
						if(*t1 == '\n')
						{
							*t2 = '\r';
							t2 = CharNext(t2);
							len++;
						}
						*t2 = *t1;
						t1 = CharNext(t1);
						t2 = CharNext(t2);
						len++;
					}

					HANDLE hMem=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,len+1);
					lstrcpyn((char*)GlobalLock(hMem),tmp2,len+1);
					GlobalUnlock(hMem);
					EmptyClipboard();
					SetClipboardData(CF_TEXT,hMem);
					CloseClipboard();
				}
			}
			break;
		case (EN_CHANGE<<16)|IDC_REFRESH:
			{
				int t=GetDlgItemInt(wnd,IDC_REFRESH,0,0);
				if (t>0)
				{
					if (t<10) t=10;
					else if (t>10000) t=10000;
					KillTimer(wnd,1);
					SetTimer(wnd,1,t,0);
					cfg_status_update_freq=t;
				}
			}
			break;
		}
		break;
	}
	return 0;
}

#ifdef HAVE_SSRC

static UINT sr_tab[]={8000,11025,16000,22050,24000,32000,44100,48000,56000,64000,88200,96000};

static const char * dither_tab[]={"none","no noise shaping","triangular spectral shape"};//,"ATH based noise shaping","ATH based noise shaping(less amplitude)"};

static const char * pdf_tab[3] = {"rectangular","triangular","gaussian"};

extern cfg_int cfg_use_resample;
extern cfg_int cfg_dither,cfg_resample_freq,cfg_resample_bps,cfg_pdf;
extern cfg_int cfg_fast;

static cfg_int cfg_resample_faqd("cfg_resample_faqd",0);

static INT_PTR CALLBACK CfgProc_SSRC(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{//resampling
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			HWND w;
			UINT n;
			w=GetDlgItem(wnd,IDC_RESAMPLE_FREQ);
			for(n=0;n<sizeof(sr_tab)/sizeof(sr_tab[0]);n++)
			{
				char foo[16];
				wsprintf(foo,"%u",sr_tab[n]);
				SendMessage(w,CB_ADDSTRING,0,(long)foo);
			}
			SetDlgItemInt(wnd,IDC_RESAMPLE_FREQ,cfg_resample_freq,0);
			
			w=GetDlgItem(wnd,IDC_RESAMPLE_BPS);
			SendMessage(w,CB_ADDSTRING,0,(long)"8");
			SendMessage(w,CB_ADDSTRING,0,(long)"16");
			SendMessage(w,CB_ADDSTRING,0,(long)"24");
			SendMessage(w,CB_SETCURSEL,(cfg_resample_bps>>3)-1,0);
			
			w=GetDlgItem(wnd,IDC_DITHER);
			for(n=0;n<3;n++)//5
			{
				SendMessage(w,CB_ADDSTRING,0,(long)dither_tab[n]);
			}
			SendMessage(w,CB_SETCURSEL,cfg_dither,0);

			w=GetDlgItem(wnd,IDC_PDF);
			for(n=0;n<3;n++)
			{
				SendMessage(w,CB_ADDSTRING,0,(long)pdf_tab[n]);
			}
			SendMessage(w,CB_SETCURSEL,cfg_pdf,0);
		}

		SendDlgItemMessage(wnd,IDC_FAST,BM_SETCHECK,cfg_fast,0);
		SendDlgItemMessage(wnd,IDC_RESAMPLE,BM_SETCHECK,cfg_use_resample,0);

		return 1;
	case WM_COMMAND:
		switch(wp)
		{
		case IDC_RESAMPLE:
			if (!cfg_resample_faqd)
			{
				MessageBox(wnd,"Resampling feature is intended only to workaround sound quality problems with certain kinds of sound hardware.\nUsing resampling on other soundcards will only lead to quality degradation (no matter what settings you use) and increased CPU usage.\nRefer to your soundcard specifications for exact info.\nIf you are not sure what to do, simply leave resampling disabled.","One-time FAQ reminder / config kiddie alert",MB_ICONWARNING);
				cfg_resample_faqd=1;
				SendMessage((HWND)lp,BM_SETCHECK,0,0);
			}
			break;
		case IDC_APPLY:
		case IDOK:
			cfg_resample_freq=GetDlgItemInt(wnd,IDC_RESAMPLE_FREQ,0,0);
			cfg_resample_bps=GetDlgItemInt(wnd,IDC_RESAMPLE_BPS,0,0);
			cfg_dither=SendDlgItemMessage(wnd,IDC_DITHER,CB_GETCURSEL,0,0);
			cfg_pdf=SendDlgItemMessage(wnd,IDC_PDF,CB_GETCURSEL,0,0);
			cfg_fast=SendDlgItemMessage(wnd,IDC_FAST,BM_GETCHECK,0,0);
			cfg_use_resample=SendDlgItemMessage(wnd,IDC_RESAMPLE,BM_GETCHECK,0,0);
			break;
		case IDCANCEL:

			break;
		}
		break;
	}
	return 0;
}


#endif


typedef struct
{
	UINT ctrl_id, dlg_id, name;
	DLGPROC proc;
} TABDESC;

static TABDESC tabs[]=
{
	{IDC_CONFIG_TAB1,IDD_CONFIG_TAB1,IDS_DEVICE,CfgProc1},
	{IDC_CONFIG_TAB2,IDD_CONFIG_TAB2,IDS_BUFFERING,CfgProc2},
	{IDC_CONFIG_TAB3,IDD_CONFIG_TAB3,IDS_FADING,CfgProc3},
	{IDC_CONFIG_TAB4,IDD_CONFIG_TAB4,IDS_OTHER,CfgProc4},
	{IDC_CONFIG_TAB6,IDD_CONFIG_TAB6,IDS_STATUS,CfgProc6},
#ifdef HAVE_SSRC
	{IDC_CONFIG_TAB_SSRC,IDD_CONFIG_TAB_SSRC,"Resampling",CfgProc_SSRC},
#endif
};

#define N_TABS (sizeof(tabs)/sizeof(tabs[0]))

static INT_PTR CALLBACK CfgProc(HWND wnd,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		config_start_time=GetTickCount();

		{
			HWND hTab=GetDlgItem(wnd,IDC_TAB);
			TC_ITEM it=
			{
				TCIF_TEXT,
				0,0,
				0,	//pszText
				0,
				-1,0
			};
			RECT r = {0};
			for(UINT n=0;n<N_TABS;n++)
			{
				it.pszText=WASABI_API_LNGSTRING(tabs[n].name);
				SendMessage(hTab,TCM_INSERTITEM,n,(LPARAM)&it);
			}
			SendMessage(hTab,TCM_SETCURFOCUS,cfg_cur_tab,0);
			GetClientRect(hTab,&r);
			TabCtrl_AdjustRect(hTab,0,&r);

			wchar_t title[128] = {0}, temp[128] = {0};
			swprintf(title,128,WASABI_API_LNGSTRINGW(IDS_PREFS_TITLE),WASABI_API_LNGSTRINGW_BUF(IDS_NULLSOFT_DS_OUTPUT_OLD,temp,128));
			SetWindowTextW(wnd,title);

			for(UINT n=0;n<N_TABS;n++)
			{
				HWND w=WASABI_API_CREATEDIALOGW(tabs[n].dlg_id,wnd,tabs[n].proc);
				SendMessage(mod.hMainWindow,WM_WA_IPC,(WPARAM)w,IPC_USE_UXTHEME_FUNC);
				SetWindowPos(w,0,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);
				SetWindowLong(w, GWL_ID, tabs[n].ctrl_id);
				ShowWindow(w,n==cfg_cur_tab ? SW_SHOW : SW_HIDE);
			}

			SetDlgItemTextW(wnd,IDC_VER,DS2_ENGINE_VER);
			SetFocus(hTab);
		}
		return 0;
	case WM_NOTIFY:
		switch(wp)
		{
		case IDC_TAB:
			if (((NMHDR*)lp)->code==TCN_SELCHANGE)
			{
				UINT n;
				HWND hTab=((NMHDR*)lp)->hwndFrom;
				cfg_cur_tab=SendMessage(hTab,TCM_GETCURSEL,0,0);
				for(n=0;n<N_TABS;n++)
				{
					HWND w=GetDlgItem(wnd,tabs[n].ctrl_id);
					ShowWindow(w,n==cfg_cur_tab ? SW_SHOW : SW_HIDE);
				}
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch(wp)
		{
		case IDC_RESET:
		{
			char warn[16];
			if (MessageBox(wnd,WASABI_API_LNGSTRING(IDS_RESET_ALL_SETTINGS_TO_DEFAULTS),
						   WASABI_API_LNGSTRING_BUF(IDS_WARNING,warn,16),MB_ICONWARNING|MB_YESNO)==IDYES)
			{
				wa2_sync_in();
				HWND hTab=GetDlgItem(wnd,IDC_TAB);
				UINT n;
				HWND w;
				RECT r;
				GetClientRect(hTab,&r);
				TabCtrl_AdjustRect(hTab,0,&r);

				for(n=0;n<N_TABS;n++)
				{
					w=GetDlgItem(wnd,tabs[n].ctrl_id);
					SendMessage(w,WM_COMMAND,IDCANCEL,0);
					DestroyWindow(w);
				}
				cfg_var::config_reset();
				for(n=0;n<N_TABS;n++)
				{
					w=WASABI_API_CREATEDIALOGW(tabs[n].dlg_id,wnd,tabs[n].proc);
					SendMessage(mod.hMainWindow,WM_WA_IPC,(WPARAM)w,IPC_USE_UXTHEME_FUNC);
					SetWindowPos(w,0,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER);
					SetWindowLong(w, GWL_ID, tabs[n].ctrl_id);
					ShowWindow(w,n==cfg_cur_tab ? SW_SHOW : SW_HIDE);
				}
				wa2_sync_out();
				}
			}
			break;
		case IDC_APPLY:
		case IDOK:
			wa2_sync_in();
			{
				UINT n;
				for(n=0;n<N_TABS;n++)
				{
					SendDlgItemMessage(wnd,tabs[n].ctrl_id,WM_COMMAND,wp,0);
				}
			}
			wa2_sync_out();
			if (wp==IDOK) EndDialog(wnd,1);
			break;
		case IDCANCEL:
			{
				UINT n;
				for(n=0;n<N_TABS;n++)
				{
					SendDlgItemMessage(wnd,tabs[n].ctrl_id,WM_COMMAND,IDCANCEL,0);
				}
			}
			
			EndDialog(wnd,0);
			break;
		}
		break;
	}
	return 0;
}

void Config(HWND w)
{
	if (WASABI_API_DIALOGBOXW(IDD_DS_CONFIG,w,CfgProc))
	{
		int maxfade=cfg_fade_firststart;
		int  f1=cfg_fade_start;
		if (f1>maxfade) maxfade=f1;
		f1=cfg_fade_seek;
		if (f1>maxfade) maxfade=f1;
		f1=cfg_fade_pause;
		if (f1>maxfade) maxfade=f1;
		if (maxfade>cfg_buf_ms)
		{
//			cfg_buf_ms=maxfade;
			char foo[256];
			wsprintf(foo,WASABI_API_LNGSTRING(IDS_SOME_FADE_TIMES_ARE_BIGGER_THAN_BUFFER),maxfade);
			if (MessageBox(w,foo,WASABI_API_LNGSTRING(IDS_WARNING),MB_ICONEXCLAMATION|MB_YESNO)==IDYES)
			{
				wa2_sync_in();
				cfg_buf_ms=maxfade;
				wa2_sync_out();
			}
		}
	}
}

class FooString//ghey. no StringPrintf().
{
private:
	char foo[32];
public:
	operator const char *() {return foo;}
	FooString(const char * s1,const char * s2)
	{
		lstrcpy(foo,s1);
		lstrcat(foo,s2);
	}
};

FadeCfg::FadeCfg(const char * _name,const char * vname,int vtime,bool von,bool vusedef)
 : time(FooString(vname,".time"),vtime)
 , on(FooString(vname,".on"),von)
 , usedef(FooString(vname,".usedef"),vusedef)
{
	name=_name;
}