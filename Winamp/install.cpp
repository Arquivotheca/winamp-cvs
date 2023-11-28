/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename: 
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "main.h"
#include "api.h"
#include "../nu/AutoChar.h"

#include "./setup/svc_setup.h"
#include <api/service/waservicefactorybase.h>
#include "./setup/postsetup.h"
#define APSTUDIO_READONLY_SYMBOLS
#include "./setup/setup_resource.h"
#include "./setup/setupfactory.h"

void DoInstall(int is_install)
{
	//2=audiotype
	//4=videotype
	//8=cd
	//16=set needreg=1
	//32=dircontextmenus
	//64=playlisttype
	//128=run setup wizard

	if (is_install&128)
	{
		HRESULT hr(S_FALSE);
		HWND hwndStatus(NULL), hwndWA(NULL);

		waServiceFactory *psf = WASABI_API_SVC->service_getServiceByGuid(UID_SVC_SETUP);
		if (psf) 
		{
			svc_setup *pSvc = (svc_setup*)psf->getInterface();
			if (pSvc)
			{
				Setup_RegisterDefault();
				Setup_RegisterPlugins();
		
				hr = pSvc->Start(hMainWindow);
						
				if (S_OK == hr) 
				{
					pSvc->CreateStatusWnd(&hwndStatus);
					if (hwndStatus)
					{
						ShowWindow(hwndStatus, SW_SHOWNORMAL);
						UpdateWindow(hwndStatus);
					}
					pSvc->Save(hwndStatus);
					pSvc->ExecJobs(hwndStatus);
				}
				pSvc->Release();
			}
			WASABI_API_SVC->service_deregister(psf);
		}

		if (S_OK == hr && hwndStatus)
		{
			SetDlgItemTextW(hwndStatus, IDC_LBL_STATUS, getStringW(IDS_STATUS_RUNWA, NULL, 0));
		}

		out_deinit();
		in_deinit();
		w5s_deinit();
		
		if (S_OK == hr)
		{
			DWORD attr = GetFileAttributesW(INI_FILE);
			if(attr & FILE_ATTRIBUTE_READONLY)
			{
				SetFileAttributesW(INI_FILE,(attr - FILE_ATTRIBUTE_READONLY));
			}

			char szWAParam[MAX_PATH];
			WritePrivateProfileStringW(L"WinampReg", L"NeedReg", L"0", INI_FILE);
			GetPrivateProfileStringA("SETUP", "WAParam", "", szWAParam, sizeof(szWAParam)/sizeof(char), INI_FILEA);
			WritePrivateProfileStringW(L"SETUP", NULL, NULL, INI_FILE);
			PathQuoteSpaces(szWAParam);
			StartWinamp((NULL != hwndStatus), &hwndWA, szWAParam);	
		}

		Wasabi_Unload();

		if (hwndStatus) DestroyWindow(hwndStatus);
		if (hwndWA && IsWindow(hwndWA)) SetForegroundWindow(hwndWA);
		RemoveRegistrar();
		ExitProcess(0);
		return;
	}

	config_setup_filetypes();
	config_registermediaplayer(1);
	
	if (is_install&32) config_adddircontext();
	else config_removedircontext();

	if (is_install&16) WritePrivateProfileStringW(L"WinampReg", L"NeedReg", L"1", INI_FILE);

	if (is_install&8)
	{
		if (!config_iscdplayer()) config_regcdplayer(1);
	}
	else
	{
		if (config_iscdplayer()) config_regcdplayer(0);
	}

	if (is_install&(4 | 2 | 64)) // audio or video or both
	{
		extern void _w_s(char *name, char *data);
		char ext_list[8192];
		ext_list[0] = 0;

		if (is_install&(4 | 2))
		{
			char *ext = in_getextlist();
			char *a = ext;

			if (ext)
			{
				while (*a)
				{
					char buf[1024];
					int type;
					char buf2[64];
					StringCchPrintf(buf, 1024, "test.%s", a);

					buf2[0] = 0;
					in_get_extended_fileinfo(buf, "type", buf2, 32);
					type = atoi(buf2);

					if (lstrlen(ext_list) + lstrlen(buf) > 8100) break;

					{
						//int isreg=!!config_isregistered(a);
						int doreg = (!type && (is_install & 2)) || (type && (is_install & 4));

						if ( /*isreg != */doreg) config_register(a, doreg);
						if (doreg)
						{
							if (*ext_list) StringCchCat(ext_list, 8192, ":");
							StringCchCat(ext_list, 8192, a);
						}
					}

					a += lstrlen(a) + 1;
				}
				GlobalFree((HGLOBAL)ext);
				SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
			}
		}

		if (is_install&64)
		{
			const wchar_t *p;
			size_t i=0;
			while (NULL != (p = playlistManager->EnumExtension(i++)))
			{
				AutoChar a(p);
				int doreg = !!(is_install & 64);
				if (doreg)
				{
					config_register(a, doreg);
					if (*ext_list) StringCchCat(ext_list, 8192, ":");
					StringCchCat(ext_list, 8192, a);
				}
				p += lstrlenW(p) + 1;
			}
		}
		_w_s("config_extlist", ext_list);
	}

	//CreateEQPresets();

	config_register("wsz", 1);
	config_register("wal", 1);
	config_register("wlz", 1);
	config_write(0);
	out_deinit();
	in_deinit();
	w5s_deinit();
	Wasabi_Unload();
	RemoveRegistrar();
	ExitProcess(0);
}