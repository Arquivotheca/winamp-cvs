#pragma comment(linker, "-nodefaultlib:libmmt.lib")
#pragma comment(linker, "-nodefaultlib:libm.lib")
#pragma message(__FILE__": telling linker to ignore libmmt.lib")

#pragma comment(linker, "-nodefaultlib:libircmt.lib")
#pragma message(__FILE__": telling linker to ignore libircmt.lib")

#pragma comment(linker, "-nodefaultlib:libirc.lib")
#pragma message(__FILE__": telling linker to ignore libircmt.lib")

#pragma comment(linker, "-nodefaultlib:svml_disp.lib")
#pragma message(__FILE__": telling linker to ignore svml_disp.lib")

#define ENC_VERSION "v1.31"

#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include <mmsystem.h>
#include "resource.h"
#include "../nsv/enc_if.h"

#include "main.h"
#include "Encoders.h"
#include "Config.h"
#include "../nu/AutoWideFn.h"
// wasabi based services for localisation support
#include <api/service/waServiceFactory.h>
#include "../Agave/Language/api_language.h"
#include "../winamp/wa_ipc.h"
#include <strsafe.h>

HWND winampwnd = 0;
api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
HINSTANCE enc_aac_plus_HINST = 0;

static char resstr[1024];
static char *getString(HINSTANCE hinst, UINT id, char *buf, int len)
{
	LoadString(hinst, id, buf, len);
	return buf;
}
#define ENC_AACPLUS_WASABI_API_LNGSTRING(uID) (WASABI_API_LNG?WASABI_API_LNGSTR(WASABI_API_LNG_HINST,WASABI_API_ORIG_HINST,uID):getString(enc_aac_plus_HINST, uID, resstr, 1024))
#define ENC_AACPLUS_WASABI_API_LNGSTRING_BUF(uID, buf, len) WASABI_API_LNG?WASABI_API_LNGSTR(WASABI_API_LNG_HINST,WASABI_API_ORIG_HINST,uID,buf,len):getString(enc_aac_plus_HINST, uID, buf, len)
#define ENC_WASABI_API_CREATEDIALOGPARAM(id, parent, proc, param) WASABI_API_LNG?WASABI_API_LNG->CreateLDialogParamW(WASABI_API_LNG_HINST, WASABI_API_ORIG_HINST, id, parent, (DLGPROC)proc, param):CreateDialogParamW(enc_aac_plus_HINST, MAKEINTRESOURCEW(id), parent, (DLGPROC)proc, param)
static HINSTANCE GetMyInstance()
{
	MEMORY_BASIC_INFORMATION mbi = {0};
	if(VirtualQuery(GetMyInstance, &mbi, sizeof(mbi)))
		return (HINSTANCE)mbi.AllocationBase;
	return NULL;
}

enum
{
	AAC_default_bitrate = 128000,
	AACPlus_default_bitrate = 64000,
	AACPlusHigh_default_bitrate = 192000,
};

void readconfig(char *configfile, char *section, AACplusConfig *cfg,  int defBitrate, aacPlusEncSbrMode aacMode, bitstreamFormat bitstream, sbrSignallingMode signallingMode)
{
	cfg->bitRate=defBitrate;
	cfg->sampleRate = 0;//44100;
	cfg->channelMode = STEREO;
	cfg->aacMode=aacMode;
	cfg->format=bitstream;
	cfg->signallingMode=signallingMode;
	cfg->speech=false;
	cfg->extraOptions=false;
	cfg->pns=false;
	lstrcpyn(cfg->section_name, section, 64);
	if (configfile) 
	{
		cfg->sampleRate=GetPrivateProfileInt(section,"samplerate",cfg->sampleRate,configfile);
		cfg->channelMode=(aacPlusEncChannelMode)GetPrivateProfileInt(section,"channelmode",cfg->channelMode,configfile);
		cfg->bitRate=GetPrivateProfileInt(section,"bitrate",cfg->bitRate,configfile);
//		cfg->format=(bitstreamFormat)GetPrivateProfileInt(section,"bitstream",cfg->format,configfile);
		cfg->speech=!!GetPrivateProfileInt(section,"speech",0,configfile);
		cfg->extraOptions=!!GetPrivateProfileInt(section,"extraOptions",0,configfile);
		//cfg->pns=(bitstreamFormat)GetPrivateProfileInt(section,"pns",0,configfile);
		//cfg->signallingMode=(sbrSignallingMode)GetPrivateProfileInt(section,"signallingmode",cfg->signallingMode,configfile);
	}
}

void GetLocalisationApiService(void)
{
	if (!enc_aac_plus_HINST)
		enc_aac_plus_HINST = GetMyInstance();

	if(winampwnd && !WASABI_API_LNG)
	{
		// loader so that we can get the localisation service api for use
		if(!WASABI_API_SVC)
		{
			WASABI_API_SVC = (api_service*)SendMessage(winampwnd, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
			if (WASABI_API_SVC == (api_service*)1)
			{
				WASABI_API_SVC = NULL;
				return;
			}
		}

		if (!WASABI_API_SVC)
			return;

		if(!WASABI_API_LNG)
		{
			waServiceFactory *sf;
			sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
			if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());
		}

		// need to have this initialised before we try to do anything with localisation features
		WASABI_API_START_LANG(GetMyInstance(),EncAACLangGUID);
	}
}

extern "C" {

	unsigned int __declspec(dllexport) GetAudioTypes3(int idx, char *desc)
	{
		GetLocalisationApiService();
		switch(idx)
		{
		case 0:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_HE_AAC_DESC), ENC_VERSION);
			return mmioFOURCC('A','A','C','P');
		case 1:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_HE_AAC_HIBR_DESC), ENC_VERSION);
			return mmioFOURCC('A','A','C','H');
		case 2:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_LC_AAC_DESC), ENC_VERSION);
			return mmioFOURCC('A','A','C','r');
		case 3:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_MP4_AAC_PLUS_DESC), ENC_VERSION);
			return mmioFOURCC('M','4','A','+');
		case 4:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_MP4_AAC_PLUS_HIBR_DESC), ENC_VERSION);
			return mmioFOURCC('M','4','A','H');
		case 5:
			StringCchPrintf(desc, 1024, ENC_AACPLUS_WASABI_API_LNGSTRING(IDS_ENC_MP4_LC_AAC_DESC), ENC_VERSION);
			return mmioFOURCC('M','4','A',' ');
		default:
			return 0;
		}
	}

	AudioCoder __declspec(dllexport)  *CreateAudio3(int nch, int srate, int bps, unsigned int srct, unsigned int *outt, char *configfile)
	{
		if (srct == mmioFOURCC('P','C','M',' '))
		{
			AACplusConfig cfg;
			AudioCoderCommon *t=0;
			switch(*outt)
			{
			case mmioFOURCC('A','A','C','P'):
				readconfig(configfile,"audio_aacplus", &cfg, AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_ADTS, IMPLICIT);
				t = new AudioCoderAACPlus(nch,srate,bps,&cfg);
				break;

			case mmioFOURCC('A','A','C','H'):
				*outt=mmioFOURCC('A','A','C','P');
				readconfig(configfile,"audio_aacplushigh", &cfg , AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_ADTS, EXPLICIT_BC);
				t = new AudioCoderAACPlusHighBitrate(nch,srate,bps,&cfg);
				break;

			case mmioFOURCC('A','A','C','r'):
				//case mmioFOURCC('A','A','C',' '):
				*outt=mmioFOURCC('A','A','C',' ');
				readconfig(configfile,"audio_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_ADTS, IMPLICIT);
				t = new AudioCoderAAC(nch,srate,bps,&cfg);
				break;

			case mmioFOURCC('M','4','A','+'):
				readconfig(configfile,"audio_mp4_aacplus", &cfg, AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_RAW, EXPLICIT_BC);
				t = new MP4CoderAACPlus(nch,srate,bps,&cfg);
				break;

				case mmioFOURCC('M','4','A','H'):
					readconfig(configfile,"audio_mp4_aacplushigh", &cfg , AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_RAW, EXPLICIT_BC);
				t = new MP4CoderAACPlusHighBitrate(nch,srate,bps,&cfg);
				break;

					case mmioFOURCC('M','4','A',' '):
					readconfig(configfile,"audio_mp4_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_RAW, EXPLICIT_BC);
				t = new MP4CoderAAC(nch,srate,bps,&cfg);
				break;
			}

			if (t && t->GetLastError())
			{
				delete t;
				t=0;
			}

			return t;
		}

		return NULL;
	}

	HWND __declspec(dllexport) ConfigAudio3(HWND hwndParent, HINSTANCE hinst, unsigned int outt, char *configfile)
	{
		GetLocalisationApiService();
		switch(outt)
		{
		case mmioFOURCC('M', '4','A','+'):
			{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_mp4_aacplus", &(wr->cfg), AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_RAW, EXPLICIT_NON_BC);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AACP_MP4,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
			break;
		case mmioFOURCC('A','A','C','P'): 
			{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_aacplus", &(wr->cfg), AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_ADTS, IMPLICIT);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AACP,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
			break;
			case mmioFOURCC('M', '4','A','H'):
				{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_mp4_aacplushigh", &(wr->cfg), AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_RAW, EXPLICIT_NON_BC);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AACH_MP4,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
				break;
		case mmioFOURCC('A','A','C','H'):
			{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_aacplushigh", &(wr->cfg), AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_ADTS, IMPLICIT);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AACH,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
			break;
			case mmioFOURCC('M','4','A',' '):
			{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_mp4_aac", &(wr->cfg), AAC_default_bitrate, SBR_OFF, BSFORMAT_RAW, EXPLICIT_BC);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AAC_MP4,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
			break;
		case mmioFOURCC('A','A','C','r'):
		case mmioFOURCC('A','A','C',' '):
			{
				ConfigWnd *wr=(ConfigWnd*)malloc(sizeof(ConfigWnd ));
				if (configfile) wr->configfile=_strdup(configfile);
				else wr->configfile=0;
				readconfig(configfile,"audio_aac", &(wr->cfg), AAC_default_bitrate, SBR_OFF, BSFORMAT_ADTS, IMPLICIT);
				return ENC_WASABI_API_CREATEDIALOGPARAM(IDD_AAC,hwndParent,AACConfigurationDialog,(LPARAM)wr);
			}
			break;
		}

		return NULL;
	}

	void __declspec(dllexport) PrepareToFinish(const char *filename, AudioCoder *coder)
	{
		((AudioCoderCommon*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) PrepareToFinishW(const wchar_t *filename, AudioCoder *coder)
	{
		((AudioCoderCommon*)coder)->PrepareToFinish();
	}

	void __declspec(dllexport) FinishAudio3(const char *filename, AudioCoder *coder)
	{
			((AudioCoderCommon*)coder)->Finish(AutoWideFn(filename));
	}

	void __declspec(dllexport) FinishAudio3W(const wchar_t *filename, AudioCoder *coder)
	{
			((AudioCoderCommon*)coder)->Finish(filename);
	}

	int __declspec(dllexport) SetConfigItem(unsigned int outt, char *item, char *data, char *configfile)
	{
		AACplusConfig cfg;
		switch(outt)
		{
		case mmioFOURCC('M','4','A','+'):
			readconfig(configfile,"audio_mp4_aacplus", &cfg, AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_RAW, EXPLICIT_NON_BC);
			break;
		case mmioFOURCC('A','A','C','P'): 
			readconfig(configfile,"audio_aacplus", &cfg, AACPlus_default_bitrate, SBR_NORMAL, BSFORMAT_ADTS, IMPLICIT);
			break;
		case mmioFOURCC('M','4','A','H'):
			readconfig(configfile,"audio_mp4_aacplushigh", &cfg, AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_RAW, EXPLICIT_NON_BC);
			break;
		case mmioFOURCC('A','A','C','H'):
			readconfig(configfile,"audio_aacplushigh", &cfg, AACPlusHigh_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_ADTS, IMPLICIT);
			break;
		case mmioFOURCC('M','4','A',' '):
			readconfig(configfile,"audio_mp4_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_RAW, EXPLICIT_BC);
			break;
		case mmioFOURCC('A','A','C','r'):
			readconfig(configfile,"audio_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_ADTS, IMPLICIT);
			break;
		default:
			return 0;
		}
		if (!lstrcmpi(item, "bitrate"))
		{
			cfg.bitRate = atoi(data)*1000;
		}
		writeconfig(configfile, &cfg);
		return 1;
	}

	int __declspec(dllexport) GetConfigItem(unsigned int outt, char *item, char *data, int len, char *configfile)
	{
		AACplusConfig cfg;
		switch(outt)
		{
		case mmioFOURCC('M','4','A','+'):
			readconfig(configfile,"audio_mp4_aacplus", &cfg, AACPlus_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_RAW, EXPLICIT_NON_BC);
			break;
		case mmioFOURCC('A','A','C','P'): 
			readconfig(configfile,"audio_aacplus", &cfg, AACPlus_default_bitrate, SBR_OVERSAMPLED, BSFORMAT_ADTS, IMPLICIT);
			break;
		case mmioFOURCC('M','4','A','H'):
			readconfig(configfile,"audio_mp4_aacplushigh", &cfg, AACPlusHigh_default_bitrate, SBR_NORMAL, BSFORMAT_RAW, EXPLICIT_NON_BC);
			break;
		case mmioFOURCC('A','A','C','H'):
			readconfig(configfile,"audio_aacplushigh", &cfg, AACPlusHigh_default_bitrate, SBR_NORMAL, BSFORMAT_ADTS, IMPLICIT);
			break;
		case mmioFOURCC('M','4','A',' '):
			readconfig(configfile,"audio_mp4_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_RAW, EXPLICIT_BC);
			break;
		case mmioFOURCC('A','A','C','r'):
			readconfig(configfile,"audio_aac", &cfg, AAC_default_bitrate, SBR_OFF, BSFORMAT_ADTS, IMPLICIT);
			break;
		default:
			return 0;
		}
		if (!lstrcmpi(item, "bitrate"))
		{
			StringCchPrintf(data,len,"%d",cfg.bitRate/1000);
		}
		else if (!lstrcmpi(item, "extension"))
		{
			if (cfg.format == BSFORMAT_ADTS)
				lstrcpynA(data, "aac", len);
			else if (cfg.format == BSFORMAT_RAW)
				lstrcpynA(data, "m4a", len);
		}
		else if (!lstrcmpi(item, "encoder"))
		{
			StringCchPrintf/*snprintf*/(data, len, "Coding Technologies' aacPlus %s", aacPlusEncGetLibraryVersion());
			data[len-1]=0;
		}
		return 1;
	}

	void __declspec(dllexport) SetWinampHWND(HWND hwnd)
	{
		winampwnd = hwnd;
	}
};