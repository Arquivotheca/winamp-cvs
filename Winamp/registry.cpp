/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
 ** Filename:
 ** Project:
 ** Description:
 ** Author:
 ** Created:
 **/

#include "Main.h"
#include "resource.h"
#include "../nu/AutoWide.h"
#include "main.hpp"
#include "api.h"

void config_registerMMS(DWORD accessEnabled)
{
	/* TODO:
	<Egg> HKEY_CLASSES_ROOT\MMS\shell\open\command
	<Egg> (Default) = "C:\Program Files\Winamp\winamp.exe" "%1"
	*/
}

/* register winamp as a media player (in "set defaults")*/
void config_registermediaplayer(DWORD accessEnabled)
{
	if (config_no_registry)
		return;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar) == 0 && registrar)
	{
		wchar_t programname[MAX_PATH];
		GetModuleFileNameW(hMainInstance,programname,MAX_PATH);

		registrar->RegisterMediaPlayer(accessEnabled, programname, AutoWide(app_name), config_whichicon);

		LPCWSTR szProtocols[] = { L"mms", L"mmsu", L"mmst", L"uvox", L"icy", L"sc", L"shout", L"unsv", };
		for (int i = 0; i < sizeof(szProtocols)/sizeof(*szProtocols); i++)
		{
			wchar_t szBuffer[64];
			StringCbPrintfW(szBuffer, sizeof(szBuffer), L"%s://", szProtocols[i]);
			int offset=0;
			if (NULL != in_setmod_noplay(szBuffer, &offset))
				registrar->RegisterMediaPlayerProtocol(szProtocols[i], AutoWide(app_name));
			else
				registrar->UnregisterMediaPlayerProtocol(szProtocols[i], AutoWide(app_name));
		}

		registrar->Release();
	}
}

void config_adddircontext(void)
{
	if (config_no_registry)
		return;
	wchar_t str[MAX_PATH+128];
	wchar_t programname[MAX_PATH];
	wchar_t langbuf[1024];
	if (!GetModuleFileNameW(hMainInstance,programname,MAX_PATH)) return;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar) == 0 && registrar)
	{
		StringCchPrintfW(str,MAX_PATH+128,L"\"%s\" \"%%1\"",programname);
		registrar->AddDirectoryContext(str, WINAMP_PLAYW, getStringW(IDS_CONFIG_PLAYSTR,langbuf,1024));

		StringCchPrintfW(str,MAX_PATH+128,L"\"%s\" /ADD \"%%1\"",programname);
		registrar->AddDirectoryContext(str, WINAMP_ENQUEUEW, getStringW(IDS_CONFIG_ENQUEUESTR,langbuf,1024));

		StringCchPrintfW(str,MAX_PATH+128,L"\"%s\" /BOOKMARK \"%%1\"",programname);
		registrar->AddDirectoryContext(str, WINAMP_BOOKMARKW, getStringW(IDS_CONFIG_BOOKMARKSTR,langbuf,1024));

		registrar->Release();
	}
}

int config_isdircontext(void)
{
	if (config_no_registry)
		return 0;
	HKEY mp3Key;
	if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Classes\\Directory\\shell\\" WINAMP_PLAY,&mp3Key) != ERROR_SUCCESS) return 0;
	RegCloseKey(mp3Key);
	return 1;
}

void config_setup_filetype(const wchar_t *winamp_file, const wchar_t *name)
{
	wchar_t programname[MAX_PATH];
	wchar_t str[MAX_PATH+32];
	wchar_t langbuf[1024];

	if (!GetModuleFileNameW(hMainInstance,programname,MAX_PATH)) return;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar) == 0 && registrar)
	{
		registrar->SetupFileType(programname, winamp_file, name, config_whichicon, config_addtolist?L"Enqueue":L"Play", L"");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, winamp_file, getStringW(IDS_CONFIG_PLAYSTR,langbuf,1024), L"Play", L"{46986115-84D6-459c-8F95-52DD653E532E}");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, winamp_file, L"", L"open", L"{46986115-84D6-459c-8F95-52DD653E532E}");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" /ADD \"%%1\"",programname);
		registrar->SetupShell(str, winamp_file, getStringW(IDS_CONFIG_ENQUEUESTR,langbuf,1024), L"Enqueue", L"{77A366BA-2BE4-4a1e-9263-7734AA3E99A2}");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" /BOOKMARK \"%%1\"",programname);
		registrar->SetupShell(str, winamp_file, getStringW(IDS_CONFIG_BOOKMARKSTR,langbuf,1024), L"ListBookmark", L"");

		registrar->Release();
	}
}

void config_setup_filetypes(void)
{
	if (config_no_registry)
		return;

	wchar_t programname[MAX_PATH];
	wchar_t str[MAX_PATH+32];
	wchar_t langbuf[1024];

	if (!GetModuleFileNameW(hMainInstance,programname,MAX_PATH)) return;

	config_setup_filetype(WINAMP_FILEW, getStringW(IDS_WINAMP_MEDIA_FILE,NULL,0));

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar) == 0 && registrar)
	{
		// droptarget stuff for windows XP
		// clsid for open
		registrar->RegisterGUID(programname, L"{46986115-84D6-459c-8F95-52DD653E532E}");
		// clsid for enqueue
		registrar->RegisterGUID(programname, L"{77A366BA-2BE4-4a1e-9263-7734AA3E99A2}");

		/* Register playlist shell type */

		registrar->RegisterTypeShell(programname, WINAMP_PLAYLISTW, getStringW(IDS_WINAMP_PLAYLIST_FILE,NULL,0), config_whichicon2, config_addtolist?L"Enqueue":L"Play");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_PLAYLISTW, getStringW(IDS_CONFIG_PLAYSTR,langbuf,1024), L"Play", L"");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_PLAYLISTW, L"", L"open", L"");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" /ADD \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_PLAYLISTW, getStringW(IDS_CONFIG_ENQUEUESTR,langbuf,1024), L"Enqueue", L"");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" /BOOKMARK \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_PLAYLISTW, getStringW(IDS_CONFIG_BOOKMARKSTR,langbuf,1024), L"ListBookmark", L"");

		/* Register skin shell type */
		registrar->RegisterTypeShell(programname, WINAMP_SKINZIPW, getStringW(IDS_WINAMP_EXTENSION_INSTALLATION_FILE,NULL,0), config_whichicon, L"Install");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_SKINZIPW, getStringW(IDS_WINAMP_FILE_INSTALL,langbuf,1024), L"Install", L"");

		/* Register language file type */
		registrar->RegisterTypeShell(programname, WINAMP_LANGZIPW, getStringW(IDS_WINAMP_LANGUAGE_INSTALLATION_FILE,NULL,0), config_whichicon, L"Install");

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" \"%%1\"",programname);
		registrar->SetupShell(str, WINAMP_LANGZIPW, getStringW(IDS_WINAMP_FILE_INSTALL,langbuf,1024), L"Install", L"");

		regmimetype(L"interface/x-winamp-skin", programname, L".wsz", 0);
		regmimetype(L"interface/x-winamp-lang", programname, L".wlz", 0);

		StringCchPrintfW(str,MAX_PATH+32,L"\"%s\" /HANDLE \"%%1\"",programname);
		wchar_t icon[MAX_PATH+32];
		StringCchPrintfW(icon,MAX_PATH+32,L"\"%s\",%d",programname, config_whichicon);
		registrar->RegisterProtocol(L"winamp", str, icon);

		//  regmimetype(L"application/x-winamp-plugin", programname,L"wpz",0);
		registrar->Release();
	}
}

// mar-17-2005-benski
void config_regdvdplayer(void)
{
	wchar_t winampPath[MAX_PATH];

	// get the full path exe
	if (!GetModuleFileNameW(hMainInstance,winampPath, MAX_PATH))
		return;

	IFileTypeRegistrar *registrar=0;
	if (GetRegistrar(&registrar) == 0 && registrar)
	{
		registrar->RegisterDVDPlayer(winampPath, config_whichicon, L"Winamp.DVD", L"Winamp", L"Nullsoft Winamp", L"Play in Winamp");
		registrar->Release();
	}
}

static void _reg_associated_filetypes(int force)
{
	char ext_list[8192], *p;
	char *c_list = in_getextlist();
	void _r_s(char *name, char *data, int mlen);
	ext_list[0] = 0;
	_r_s("config_extlist", ext_list, sizeof(ext_list));
	p = ext_list;
	int something_regged=0;
	while (*p)
	{
		char *p2 = p;
		while (*p2 && *p2 != ':') p2++;
		if (*p2 == ':') *p2++ = 0;
		if (!config_isregistered(p))
		{
			// dropped register on startup for Win8 as we cannot
			// re-associate due to changes in the OSes behaviour
			if (force || (!IsWin8() && config_check_ft_startup))
			{
				config_register(p, 1);
				something_regged=1;
			}
		}
		else
		{
			int a = 0;
			char *w = c_list;
			if (IsPlaylistExtensionA(p))
				a = 1;
			else while (*w && !a)
				{
					if (!_stricmp(w, p)) a = 1;
					w += lstrlen(w) + 1;
				}
			if (!a)
			{
				config_register(p, 0);
				something_regged=1;
			}
		}
		p = p2;
	}
	GlobalFree((HGLOBAL)c_list);
	if (something_regged)
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
}

static int RegisterThread(HANDLE handle, void *user_data, intptr_t id)
{
	CoInitializeEx(0,COINIT_MULTITHREADED);
	_reg_associated_filetypes((int)id);
	CoUninitialize();
	return 0;
}

void reg_associated_filetypes(int force)
{
	WASABI_API_THREADPOOL->RunFunction(0, RegisterThread, 0, force, api_threadpool::FLAG_LONG_EXECUTION|api_threadpool::FLAG_REQUIRE_COM_MT);
}