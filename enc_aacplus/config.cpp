#include "../Agave/Language/api_language.h"
#include "Config.h"
#include "resource.h"
#include <shellapi.h>
#include <malloc.h>
#include "../winamp/wa_ipc.h"
extern HINSTANCE enc_aac_plus_HINST;
static wchar_t resstr[1024];
static wchar_t *getString(HINSTANCE hinst, UINT id, wchar_t *buf, int len)
{
	LoadStringW(hinst, id, buf, len);
	return buf;
}
#define ENC_AACPLUS_WASABI_API_LNGSTRING(uID) (WASABI_API_LNG?WASABI_API_LNGSTRW(WASABI_API_LNG_HINST,WASABI_API_ORIG_HINST,uID):getString(enc_aac_plus_HINST, uID, resstr, 1024))
#define ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(uID, buf, len) WASABI_API_LNG?WASABI_API_LNGSTRW(WASABI_API_LNG_HINST,WASABI_API_ORIG_HINST,uID,buf,len):getString(enc_aac_plus_HINST, uID, buf, len)

static bool HasChannelMode(HWND hwndDlg, int item, int mode)
{
	int numChannelModes = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT,0,0);
	for (int i=0;i<numChannelModes;i++)
	{
		if (SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0) == mode)
			return true;
	}
	return false;
}

int GetBitRate(HWND hwndDlg)
{
	return SendDlgItemMessage(hwndDlg, IDC_BITRATE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg,IDC_BITRATE,CB_GETCURSEL,0,0), 0);
}

sbrSignallingMode GetSignallingMode(HWND hwndDlg)
{
	return (sbrSignallingMode)SendDlgItemMessage(hwndDlg, IDC_SIGNALLING, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg,IDC_SIGNALLING,CB_GETCURSEL,0,0), 0);
}

aacPlusEncChannelMode GetChannelMode(HWND hwndDlg)
{
	return (aacPlusEncChannelMode)SendDlgItemMessage(hwndDlg, IDC_CHANNELMODE, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg,IDC_CHANNELMODE, CB_GETCURSEL,0,0), 0);
}

bool SelectBitRate(HWND hwndDlg, int item, int bitRate)
{
	SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, 0, 0);
	int numBitRates = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT,0,0);
	for (int i=0;i<numBitRates;i++)
	{
		if (SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0) == bitRate)
		{
			SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, i, 0);
			return true;
		}
	}
	return false;
}

bool HasBitRate(HWND hwndDlg, int item, int bitRate)
{
	SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, 0, 0);
	int numBitRates = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT,0,0);
	for (int i=0;i<numBitRates;i++)
	{
		if (SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0) == bitRate)
		{
			return true;
		}
	}
	return false;
}

bool SelectChannelMode(HWND hwndDlg, int item, int mode)
{
	int numChannelModes = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT,0,0);
	for (int i=0;i<numChannelModes;i++)
	{
		if (SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0) == mode)
		{
			SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, i, 0);
			return true;
		}
	}
	return false;
}

void writeconfig(char *configfile, AACplusConfig *cfg)
{
	if(configfile)
	{
		char tmp[64];
		wsprintf(tmp, "%d", cfg->sampleRate);
		WritePrivateProfileString(cfg->section_name, "samplerate", tmp, configfile);
		wsprintf(tmp, "%d", cfg->channelMode);
		WritePrivateProfileString(cfg->section_name, "channelmode", tmp, configfile);
		wsprintf(tmp, "%d", cfg->bitRate);
		WritePrivateProfileString(cfg->section_name, "bitrate", tmp, configfile);
		wsprintf(tmp, "%d", cfg->format);
		WritePrivateProfileString(cfg->section_name, "bitstream", tmp, configfile);
		wsprintf(tmp, "%d", cfg->signallingMode);
		WritePrivateProfileString(cfg->section_name, "signallingmode", tmp, configfile);
		wsprintf(tmp, "%d", cfg->speech?1:0);
		WritePrivateProfileString(cfg->section_name, "speech", tmp, configfile);
		wsprintf(tmp, "%d", cfg->pns?1:0);
		WritePrivateProfileString(cfg->section_name, "pns", tmp, configfile);
	}
}

void AddBitrateSorted(HWND hwndDlg, int item, int bitrate)
{
	wchar_t tmp[64];
	SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, 0, 0);
	int numBitRates = SendDlgItemMessage(hwndDlg, item, CB_GETCOUNT,0,0);
	for (int i=0;i<numBitRates;i++)
	{
		int thisBitrate=SendDlgItemMessage(hwndDlg, item, CB_GETITEMDATA, i, 0);
		if (thisBitrate == bitrate) // duplicate, let's bail
			return;
		else if (thisBitrate<bitrate)
		{
			wsprintfW(tmp,L"%d %s", bitrate/1000, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_KBPS));
			int newpos=SendDlgItemMessageW(hwndDlg,item,CB_INSERTSTRING , i, (LPARAM)tmp);
			SendDlgItemMessage(hwndDlg,item,CB_SETITEMDATA, newpos, bitrate);
			return;
		}
	}
	wsprintfW(tmp,L"%d %s", bitrate/1000, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_KBPS));
	int newpos=SendDlgItemMessageW(hwndDlg,item,CB_INSERTSTRING , -1, (LPARAM)tmp);
	SendDlgItemMessage(hwndDlg,item,CB_SETITEMDATA, newpos, bitrate);
}

void FillBitrates(HWND hwndDlg, int item, aacPlusEncFormatList *formatList)
{

	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	int i=0;
	for (int format=0;format < formatList->numEntries;format++)
	{
		//if (formatList->format[format].sbrRateMode != wc->cfg.sbr)
		//continue;
		//if (formatList->format[format].sampleRate != wc->cfg.sampleRate)
			//continue;
		//if (HasBitRate(hwndDlg, item, formatList->format[format].bitRate))
//			continue;
		AddBitrateSorted(hwndDlg, item, formatList->format[format].bitRate);

		//if (formatList->preferredFormatIndex == format)
			//SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, i, 0);
		i++;
	}
}

void BuildChannelComboBox(HWND hwndDlg, int item)
{
	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	wchar_t tmp[64];
	int i=0;
	SendDlgItemMessage(hwndDlg,item, CB_RESETCONTENT,0, 0);

	aacPlusEncOutputFormat seedFormat;
	memset(&seedFormat, 0, sizeof(seedFormat));
	seedFormat.sbrMode=wc->cfg.aacMode;
	seedFormat.bitRate=wc->cfg.bitRate;

	aacPlusEncFormatList *formatList=aacPlusEncGetFormatList(wc->configEncoder,seedFormat,1,0, APELIST_SORT_TOP_DOWN);
	for (int format=0;format < formatList->numEntries;format++)
	{
		if (HasChannelMode(hwndDlg, item, formatList->format[format].channelMode))
			continue;
		switch(formatList->format[format].channelMode)
		{
		case MONO:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_MONO,tmp,64);
			break;
		case STEREO:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_STEREO,tmp,64);
			break;
		case STEREO_INDEPENDENT:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_INDEPENDENT_STEREO,tmp,64);
			break;
		case PARAMETRIC_STEREO:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_PARAMETRIC_STEREO,tmp,64);
			break;
		case DUAL_CHANNEL:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_DUAL_CHANNEL,tmp,64);
			break;
		case MODE_4_CHANNEL_2CPE:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_4_CHANNEL_2CPE,tmp,64);
			break;
		case MODE_4_CHANNEL_MPEG:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_4_CHANNEL_MPEG,tmp,64);
			break;
		case MODE_5_CHANNEL:
			ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(IDS_5_CHANNEL_SURROUND,tmp,64);
			break;
		case MODE_5_1_CHANNEL:
			lstrcpyW(tmp, L"5.1");
			break;
		case MODE_6_1_CHANNEL:
			lstrcpyW(tmp, L"6.1");
			break;
		case MODE_7_1_CHANNEL:
			lstrcpyW(tmp, L"7.1");
			break;
		}
		SendDlgItemMessageW(hwndDlg,item,CB_ADDSTRING,0, (LPARAM)tmp);
		SendDlgItemMessage(hwndDlg,item,CB_SETITEMDATA, i, formatList->format[format].channelMode);
		if (formatList->preferredFormatIndex==format
			|| (formatList->preferredFormatIndex=-1 && i==0))
			SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, i, 0);
		i++;
	}
}


void BuildBitRates(HWND hwndDlg, int item)
{
ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	SendDlgItemMessage(hwndDlg,item, CB_RESETCONTENT,0, 0);

	aacPlusEncOutputFormat seedFormat;
	memset(&seedFormat, 0, sizeof(seedFormat));
	seedFormat.sbrMode=wc->cfg.aacMode;
	
	aacPlusEncFormatList *formatList=aacPlusEncGetFormatList(wc->configEncoder,seedFormat,1,0, APELIST_SORT_TOP_DOWN);
	if (formatList)
		FillBitrates(hwndDlg, item, formatList);
}

void FillSignalling(HWND hwndDlg, int item)
{
ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	SendDlgItemMessage(hwndDlg,item, CB_RESETCONTENT,0, 0);

	SendDlgItemMessage(hwndDlg,item,CB_ADDSTRING,0, (LPARAM)ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ACCURATE_INFORMATION));
	SendDlgItemMessage(hwndDlg,item,CB_SETITEMDATA, 0, EXPLICIT_NON_BC);
	if (wc->cfg.signallingMode == EXPLICIT_NON_BC
		|| wc->cfg.signallingMode == IMPLICIT)
		SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, 0, 0);

	SendDlgItemMessage(hwndDlg,item,CB_ADDSTRING,0, (LPARAM)ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_BACKWARDS_COMPATIBLE));
	SendDlgItemMessage(hwndDlg,item,CB_SETITEMDATA, 1, EXPLICIT_BC);
	if (wc->cfg.signallingMode == EXPLICIT_BC)
		SendDlgItemMessage(hwndDlg,item, CB_SETCURSEL, 1, 0);

}

void BuildChoices(HWND hwndDlg, AACplusConfig *cfg)
{
		BuildBitRates(hwndDlg, IDC_BITRATE);
		SelectBitRate(hwndDlg, IDC_BITRATE, cfg->bitRate);
		cfg->bitRate = GetBitRate(hwndDlg);
		BuildChannelComboBox(hwndDlg, IDC_CHANNELMODE);
		SelectChannelMode(hwndDlg, IDC_CHANNELMODE, cfg->channelMode);

		if (cfg->format == BSFORMAT_ADTS)
			CheckDlgButton(hwndDlg, IDC_MPEG2, BST_CHECKED);
		else if (cfg->format == BSFORMAT_ADTS_MP4)
			CheckDlgButton(hwndDlg, IDC_MPEG4, BST_CHECKED);

		FillSignalling(hwndDlg, IDC_SIGNALLING);

		// get config settings from UI and write out to disk - the incoming settings may have been invalid
		cfg->channelMode = GetChannelMode(hwndDlg);
		cfg->bitRate = GetBitRate(hwndDlg);
		
		CheckDlgButton(hwndDlg, IDC_SPEECH, cfg->speech?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_PNS, cfg->pns?BST_CHECKED:BST_UNCHECKED);
}

static HCURSOR link_hand_cursor;
LRESULT link_handlecursor(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = CallWindowProcW((WNDPROC)GetPropW(hwndDlg, L"link_proc"), hwndDlg, uMsg, wParam, lParam);
	// override the normal cursor behaviour so we have a hand to show it is a link
	if(uMsg == WM_SETCURSOR)
	{
		if((HWND)wParam == hwndDlg)
		{
			if(!link_hand_cursor)
			{
				link_hand_cursor = LoadCursor(NULL, IDC_HAND);
			}
			SetCursor(link_hand_cursor);
			return TRUE;
		}
	}
	return ret;
}

void link_startsubclass(HWND hwndDlg, UINT id)
{
HWND ctrl = GetDlgItem(hwndDlg, id);
	if(!GetPropW(ctrl, L"link_proc"))
	{
		SetPropW(ctrl, L"link_proc",
				(HANDLE)SetWindowLongPtrW(ctrl, GWLP_WNDPROC, (LONG_PTR)link_handlecursor));
	}
}

BOOL CALLBACK AACConfigurationDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam) 
{
	ConfigWnd *wc = (ConfigWnd *)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	if (uMsg == WM_INITDIALOG)
	{
		if (!lParam) // this should NEVER happen
			return 0;
		SetWindowLongPtr(hwndDlg,GWLP_USERDATA,lParam);
		wc=(ConfigWnd*)lParam;
		// TODO: we should really change the API To pass the info on the commandline
		wc->configEncoder = aacPlusEncOpen(44100, 2, aacPlusEncInputShort,1,1);
		if (!wc->configEncoder)
			return 0;
		BuildChoices(hwndDlg, &(wc->cfg));
		writeconfig(wc->configfile, &wc->cfg);
		// this will make sure that we've got thr aacplus logo shown even when using a localised version
		SendDlgItemMessage(hwndDlg,IDC_LOGO,STM_SETIMAGE,IMAGE_BITMAP,
			(LPARAM)LoadImage(WASABI_API_ORIG_HINST?WASABI_API_ORIG_HINST:enc_aac_plus_HINST,MAKEINTRESOURCE(IDB_AACLOGO),IMAGE_BITMAP,0,0,LR_SHARED));

		link_startsubclass(hwndDlg, IDC_LOGO);
		return 0;
	}

	if(uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
		case IDC_CHANNELMODE:
			if (HIWORD(wParam) ==  CBN_SELCHANGE)
			{
				wc->cfg.channelMode = GetChannelMode(hwndDlg);
				writeconfig(wc->configfile,&wc->cfg);
			}
			break;

		case IDC_BITRATE:
			if ( HIWORD(wParam) ==  CBN_SELCHANGE )
			{
				wc->cfg.bitRate = GetBitRate(hwndDlg);
				BuildChannelComboBox(hwndDlg, IDC_CHANNELMODE);
				SelectChannelMode(hwndDlg, IDC_CHANNELMODE, wc->cfg.channelMode);
				wc->cfg.channelMode = GetChannelMode(hwndDlg);
				writeconfig(wc->configfile,&wc->cfg);
			}
			break;

		case IDC_SIGNALLING:
			if ( HIWORD(wParam) ==  CBN_SELCHANGE )
			{
				wc->cfg.signallingMode = GetSignallingMode(hwndDlg);
				writeconfig(wc->configfile,&wc->cfg);
			}
			break;

		case IDC_MPEG4:
			wc->cfg.format=BSFORMAT_ADTS_MP4;
			writeconfig(wc->configfile, &wc->cfg);
			break;

		case IDC_MPEG2:
			wc->cfg.format=BSFORMAT_ADTS;
			writeconfig(wc->configfile, &wc->cfg);
			break;

		case IDC_LOGO:
			SendMessage(winampwnd, WM_WA_IPC, (WPARAM)"http://www.aacplus.net/", IPC_OPEN_URL);
			break;

		case IDC_SPEECH:
			wc->cfg.speech=IsDlgButtonChecked(hwndDlg, IDC_SPEECH)?true:false;
			break;

		case IDC_PNS:
			wc->cfg.pns=IsDlgButtonChecked(hwndDlg, IDC_PNS)?true:false;
			break;

		default:
			break;
		}
	}
	if (uMsg == WM_DESTROY)
	{
		wc=(ConfigWnd*)SetWindowLongPtr(hwndDlg,GWLP_USERDATA,0);
		if (wc)
		{
			wc->cfg.signallingMode = GetSignallingMode(hwndDlg);
			wc->cfg.bitRate=GetBitRate(hwndDlg);
			wc->cfg.channelMode = GetChannelMode(hwndDlg);
			writeconfig(wc->configfile,&wc->cfg);
			free(wc->configfile);
			aacPlusEncClose(wc->configEncoder);
			wc->configEncoder=0;
			free(wc);
		}
	}
	return 0;
}