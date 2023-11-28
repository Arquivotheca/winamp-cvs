/** (c) Nullsoft, Inc.         C O N F I D E N T I A L
** Filename: 
** Project:
** Description:
** Author:
** Created:
**/

#include "main.h"
#include "resource.h"
#include "strutil.h"
#include "../nu/AutoChar.h"
#include "../nu/AutoWide.h"
#include "../nu/ns_wc.h"
#include "api.h"
#include "main.hpp"
#include "MergePlaylist.h"

/*
TODO: maybe we could make some nice little commandline helper thing
*/
const wchar_t help[] = L"/NEW\n"
					   L"/REG=\n"
					   L"/NOREG\n"
					   L"/UNREG\n"
					   L"/ADD\n"
					   L"/BOOKMARK\n"
					   L"/CONFIG=\n"
					   L"/INIDIR=\n"
					   L"/M3UDIR=\n"
					   L"/CLASS=\n"
					   L"/DELM3U\n"
					   L"/CLOSE\n"
					   L"/KILL\n"
					   L"/RETURN=\n"
					   L"/BURN=\n"
					   L"/UNINSTALL\n"
					   L"/SAFE=\n"
;

static HWND find_otherwinamp_fast()
{
	HANDLE waitEvent;
	wchar_t buf[MAX_PATH];
	StringCchPrintfW(buf, MAX_PATH, L"%s_%x_CLASS", szAppName, APP_VERSION_NUM);
	waitEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, buf);
	if (waitEvent)
	{
		HWND lhwnd = 0;
		CloseHandle(waitEvent);
		while (NULL != (lhwnd = FindWindowExW(NULL, lhwnd, szAppName, NULL)))
		{
			if (lhwnd != hMainWindow)
				return lhwnd;
		}
	}
	return NULL;
}

wchar_t *EatSpaces(wchar_t *cmdLine)
{
	while (*cmdLine && *cmdLine == L' ')
		cmdLine = CharNextW(cmdLine);

	return cmdLine;
}

static const wchar_t *EatSpaces(const wchar_t *cmdLine)
{
	while (*cmdLine && *cmdLine == L' ')
		cmdLine = CharNextW(cmdLine);

	return cmdLine;
}

wchar_t *FindNextCommand(wchar_t *cmdLine)
{
	int inQuotes = 0;
	while (*cmdLine)
	{
		if (*cmdLine == L' ' && !inQuotes) // if we see a space (and we're not in quotes) then we're done
		{
			// we purposefully don't eat any extra space characters here
			// that way we can null terminate the results of this function and get a clean string
			break;
		}
		else if (*cmdLine == L'\"') // check for quotes
		{
			inQuotes = !inQuotes; // toggles quotes mode
		}
		cmdLine = CharNextW(cmdLine); // iterate the string
	}
	return cmdLine;
}

void GetParameter(const wchar_t *commandLine, wchar_t *yourBuffer, size_t yourBufferSize)
{
	int inQuotes = 0;

	commandLine = EatSpaces(commandLine);

	for(;;)
	{
		if (yourBufferSize == 1  // buffer is out
			|| *commandLine == 0 // out of stuff to copy
			|| (*commandLine == L' ' && !inQuotes)) // or we found a space
		{
			*yourBuffer = 0;
			break;
		}
		else if (*commandLine == L'\"') // don't copy quotes
		{
			inQuotes = !inQuotes; // but do toggle the quote flag (so we can ignore spaces)
		}
		else // safe character to copy
		{
			*yourBuffer++ = *commandLine;
			yourBufferSize--;
		}
		commandLine++;
	}
}

bool IsCommand(const wchar_t *cmdline, const wchar_t *str, size_t size)
{
	if (!_wcsnicmp(cmdline, str, size) && (!cmdline[size] || cmdline[size] == L' '))
		return true;
	else
		return false;
}

static void createPlayListDBFileName(wchar_t *filename)
{
	wchar_t *filenameptr;
	int x = 32;
	for (;;)
	{
		GetTempFileNameW(M3UDIR, L"plf", GetTickCount() + x*5000, filename);
		if (lstrlenW(filename) > 4)
		{
			PathRemoveExtensionW(filename);
			PathAddExtensionW(filename, L".m3u8");
		}
		HANDLE h = CreateFileW(filename, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW, 0, 0);
		if (h != INVALID_HANDLE_VALUE)
		{
			filenameptr = filename + lstrlenW(M3UDIR) + 1;
			CloseHandle(h);
			break;
		}
		if (++x > 4096)
		{
			filenameptr = L"error.m3u";
			break;
		}
	}
}

wchar_t *ParseParameters(wchar_t *lpszCmdParam, int *bAdd, int *bBookmark, int *bHandle)
{
	for (;;)
	{
		lpszCmdParam = EatSpaces(lpszCmdParam);
		if (IsCommand(lpszCmdParam, L"-embedding", 10))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 10);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/SAFE=", 6))
		{
			wchar_t p[1024] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 6);
			GetParameter(lpszCmdParam, p, 1024);
			int mode = _wtoi(p);

			g_safeMode = (1 + (mode == 2));
		}
		else if (IsCommand(lpszCmdParam, L"/?", 2))
		{
			MessageBoxW(NULL, help, L"Winamp Commandline", MB_OK);
			// need to see if we need to update this or not due to when it's run in the startup process
			//MessageBoxW(NULL, help, getStringW(IDS_WINAMP_CMDLINE,NULL,0), MB_OK);
			ExitProcess(0);
		}
		else if (IsCommand(lpszCmdParam, L"/NEW", 4))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 4);
			bNoHwndOther = 1;
		}
		else if (IsCommand(lpszCmdParam, L"/HANDLE", 7))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 7);
			*bHandle = 1;
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/REG=", 5))
		{
			wchar_t p[1024] = {0};
			wchar_t *pItr = p;
			lpszCmdParam = SkipXW(lpszCmdParam, 5);
			GetParameter(lpszCmdParam, p, 1024);

			is_install = 1;
			while (*pItr)
			{
				// changed 5.7 - cope with upper + lowercase
				// if the commands are done that way as well
				switch (*pItr)
				{
					case L'A': case L'a': is_install |= 2; break; //2=audiotype
					case L'V': case L'v': is_install |= 4; break; //4=videotype
					case L'C': case L'c': is_install |= 8; break; //8=cd
					case L'N': case L'n': is_install |= 16; break; //16=set needreg=1
					case L'D': case L'd': is_install |= 32; break; //32=dircontextmenus
					case L'L': case L'l': is_install |= 64; break; //64=playlisttype
					case L'S': case L's': is_install |= 128; break; //128=setupwizard
				}
				pItr = CharNextW(pItr);
			}
		}
		else if (IsCommand(lpszCmdParam, L"/NOREG", 6))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 6);
			g_noreg = 1;
		}
		else if (IsCommand(lpszCmdParam, L"/UNREG", 6))
		{
			char ext_list[8192];
			char *a = ext_list;
			void _r_s(char *name, char *data, int mlen);
			CoInitialize(0);
			setup_config();
			Wasabi_Load();
			w5s_init();

			ext_list[0] = 0;
			_r_s("config_extlist", ext_list, sizeof(ext_list));
			while (a && *a)
			{
				char *p = strstr(a, ":");
				if (p) *p++ = 0;
				config_register(a, 0);
				a = p;
			}

			wchar_t playlistExtensions[1024];
			playlistManager->GetExtensionList(playlistExtensions, 1024);
			wchar_t *p = playlistExtensions;
			while (*p)
			{
				config_register(AutoChar(p), 0);
				p += lstrlenW(p) + 1;
			}

			a = "wsz\0wpz\0wal\0wlz\0";
			while (*a)
			{
				config_register(a, 0);
				a += lstrlen(a) + 1;
			}
			config_regcdplayer(0);
			if (config_isdircontext()) config_removedircontext();
			config_registermediaplayer(0);
			w5s_deinit();
			Wasabi_Unload();
			SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, NULL, NULL);
			RemoveRegistrar();
			ExitProcess(0);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/ADD", 4) && (!lpszCmdParam[4] || lpszCmdParam[4] == L' '))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 4);
			*bAdd = 1;
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/ADDPLAYLIST", 12) && (!lpszCmdParam[12] || lpszCmdParam[12] == L' '))
		{
			// winamp.exe /ADDPLAYLIST playlist.m3u "Playlist Name" {GUID}
			lpszCmdParam = SkipXW(lpszCmdParam, 12);
			setup_config();
			Wasabi_Load();
			w5s_init();
			if (AGAVE_API_PLAYLISTS)
			{
				config_read(1);
				wchar_t playlist_filename[MAX_PATH];
				wchar_t playlist_name[256];
				wchar_t playlist_guid_str[256];
				GUID playlist_guid = INVALID_GUID;
				GetParameter(lpszCmdParam, playlist_filename, MAX_PATH);
				if (playlist_filename[0])
				{
					lpszCmdParam = EatSpaces(lpszCmdParam);
					lpszCmdParam = FindNextCommand(lpszCmdParam);
					GetParameter(lpszCmdParam, playlist_name, 256);

					lpszCmdParam = EatSpaces(lpszCmdParam);
					lpszCmdParam = FindNextCommand(lpszCmdParam);
					GetParameter(lpszCmdParam, playlist_guid_str, 256);

					if (playlist_name[0] == 0)
						StringCchCopyW(playlist_name, 256, PathFindFileNameW(playlist_filename));
					if (playlist_guid_str[0] != 0)
					{
						int skip = playlist_guid_str[0] == L'{';
						playlist_guid_str[37]=0;
						UuidFromStringW((RPC_WSTR)(&playlist_guid_str[skip]), (UUID *)&playlist_guid);
					}

					AGAVE_API_PLAYLISTS->AddPlaylist(playlist_filename, playlist_name, playlist_guid);
					AGAVE_API_PLAYLISTS->Flush();
					w5s_deinit();
					Wasabi_Unload();
					ExitProcess(0);
				}
			}
			w5s_deinit();
			Wasabi_Unload();
			ExitProcess(1);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/CREATEPLAYLIST", 15) && (!lpszCmdParam[15] || lpszCmdParam[15] == L' '))
		{
			// winamp.exe /CREATEPLAYLIST "Playlist Name" {GUID}
			lpszCmdParam = SkipXW(lpszCmdParam, 15);
			setup_config();
			Wasabi_Load();
			w5s_init();
			if (AGAVE_API_PLAYLISTS)
			{
				config_read(1);
				wchar_t playlist_name[256];
				wchar_t playlist_guid_str[256];
				GUID playlist_guid = INVALID_GUID;
				GetParameter(lpszCmdParam, playlist_name, 256);
				if (playlist_name[0])
				{
					wchar_t playlist_filename[MAX_PATH];
					lpszCmdParam = EatSpaces(lpszCmdParam);
					lpszCmdParam = FindNextCommand(lpszCmdParam);
					GetParameter(lpszCmdParam, playlist_guid_str, 256);

					if (playlist_guid_str[0] != 0)
					{
						int skip = playlist_guid_str[0] == L'{';
						playlist_guid_str[37]=0;
						UuidFromStringW((RPC_WSTR)(&playlist_guid_str[skip]), (UUID *)&playlist_guid);
					}
					if (playlist_guid != INVALID_GUID)
					{
						size_t existing_playlist_index;
						// check for duplicate GUID
						if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &existing_playlist_index) != API_PLAYLISTS_SUCCESS)
						{
							createPlayListDBFileName(playlist_filename); // generate filename
							AGAVE_API_PLAYLISTS->AddPlaylist(playlist_filename, playlist_name, playlist_guid);
							AGAVE_API_PLAYLISTS->Flush();
						}
					}
					w5s_deinit();
					Wasabi_Unload();
					ExitProcess(0);
				}
			}
			w5s_deinit();
			Wasabi_Unload();
			ExitProcess(1);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/APPENDPLAYLIST", 15) && (!lpszCmdParam[15] || lpszCmdParam[15] == L' '))
		{
			// winamp.exe /APPENDPLAYLIST {GUID} filename.mp3
			lpszCmdParam = SkipXW(lpszCmdParam, 15);
			setup_config();
			Wasabi_Load();
			w5s_init();
			if (AGAVE_API_PLAYLISTS && AGAVE_API_PLAYLISTMANAGER)
			{
				config_read(1);
				wchar_t playlist_guid_str[256];
				GUID playlist_guid = INVALID_GUID;
				GetParameter(lpszCmdParam, playlist_guid_str, 256);
				if (playlist_guid_str[0])
				{
					wchar_t filename[MAX_PATH];
					const wchar_t *playlist_filename;
					lpszCmdParam = EatSpaces(lpszCmdParam);
					lpszCmdParam = FindNextCommand(lpszCmdParam);
					GetParameter(lpszCmdParam, filename, MAX_PATH);

					int skip = playlist_guid_str[0] == L'{';
					playlist_guid_str[37]=0;
					UuidFromStringW((RPC_WSTR)(&playlist_guid_str[skip]), (UUID *)&playlist_guid);

					MergePlaylist merged_playlist;

					size_t playlist_index;
					// get playlist filename from AGAVE_API_PLAYLISTS
					if (AGAVE_API_PLAYLISTS->GetPosition(playlist_guid, &playlist_index) == API_PLAYLISTS_SUCCESS
						&& (NULL != (playlist_filename = AGAVE_API_PLAYLISTS->GetFilename(playlist_index))))
					{
						// load playlist into merge_playlist
						if (AGAVE_API_PLAYLISTMANAGER->Load(playlist_filename, &merged_playlist) == PLAYLISTMANAGER_SUCCESS)
						{
							MergePlaylist appended_playlist;
							// if filename is a playlist, load it
							if (AGAVE_API_PLAYLISTMANAGER->Load(filename, &appended_playlist) == PLAYLISTMANAGER_SUCCESS)
							{
								merged_playlist.AppendPlaylist(appended_playlist);
							}
							else if (PathIsDirectoryW(filename))
							{ // if it's a directory
								AGAVE_API_PLAYLISTMANAGER->LoadDirectory(filename, &appended_playlist, 0);
								merged_playlist.AppendPlaylist(appended_playlist);
							}
							else
							{
								// TODO: get metadata, but we don't have any plugins loaded
								if (!merged_playlist.HasFilename(filename))
									merged_playlist.AppendWithInfo(filename, 0, -1);
							}
							if (AGAVE_API_PLAYLISTMANAGER->Save(playlist_filename, &merged_playlist) == PLAYLISTMANAGER_SUCCESS)
							{
								size_t num_items = merged_playlist.GetNumItems();
								AGAVE_API_PLAYLISTS->SetInfo(playlist_index, api_playlists_itemCount, &num_items, sizeof(num_items));
								uint64_t total_time = merged_playlist.total_time/1000ULL;
								AGAVE_API_PLAYLISTS->SetInfo(playlist_index, api_playlists_totalTime, &total_time, sizeof(total_time));
								AGAVE_API_PLAYLISTS->Flush();
							}							
						}
					}
					w5s_deinit();
					Wasabi_Unload();
					ExitProcess(0);
				}
			}
			w5s_deinit();
			Wasabi_Unload();
			ExitProcess(1);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/BOOKMARK", 9) && (!lpszCmdParam[9] || lpszCmdParam[9] == L' '))
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 9);
			*bBookmark = 1;
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/CONFIG=", 8))
		{
			wchar_t p[1024] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 8);
			GetParameter(lpszCmdParam, p, 1024);
			config_setinifile(p);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/INIDIR=", 8))
		{
			wchar_t p[1024] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 8);
			GetParameter(lpszCmdParam, p, 1024);
			config_setinidir(p);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/M3UDIR=", 8))
		{
			wchar_t p[1024] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 8);
			GetParameter(lpszCmdParam, p, 1024);
			config_setm3udir(p);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/CLASS=", 7))
		{
			wchar_t p[1024] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 7);
			GetParameter(lpszCmdParam, p, 1024);
			StringCchCopyW(szAppName, 64, p);
		}
		else if (IsCommand(lpszCmdParam, L"/DELM3U", 7))
		{
			setup_config();
			//if (MessageBox(NULL, "Do you want to keep your Winamp playlist?", "Removing Winamp Files ...", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
			{
				DeleteFileW(M3U_FILE);
				DeleteFileW(OLD_M3U_FILE);
			}
			RemoveRegistrar();
			ExitProcess(0);
		}
		else if (IsCommand(lpszCmdParam, L"/CLOSE", 6))
		{
			HWND hwnd_other_winamp;
			lpszCmdParam = SkipXW(lpszCmdParam, 5);
			hwnd_other_winamp = find_otherwinamp_fast();
			if (hwnd_other_winamp)
				PostMessage(hwnd_other_winamp, WM_CLOSE, 0, 0);
			ExitProcess(0);
		}
		else if (IsCommand(lpszCmdParam, L"/KILL", 5))
		{
			HWND hwnd_other_winamp;
			DWORD other_winamp_procId = 0;
			DWORD exitCode = 0;
			lpszCmdParam = SkipXW(lpszCmdParam, 5);
			hwnd_other_winamp = find_otherwinamp_fast();
			if (hwnd_other_winamp)
			{
				PostMessage(hwnd_other_winamp, WM_CLOSE, 0, 0);

				GetWindowThreadProcessId(hwnd_other_winamp, &other_winamp_procId); // get the process ID
				if (other_winamp_procId) // if we didn't get one, it probably already handled the WM_CLOSE message ...
				{
					HANDLE other_winamp_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE, FALSE, other_winamp_procId);
					if (other_winamp_process) // if we got a process handle (it might have quit already in the meantime ...)
					{
						if (WaitForSingleObject(other_winamp_process, 3000) == WAIT_TIMEOUT) // wait 5 seconds for it to close
						{
							TerminateProcess(other_winamp_process, exitCode); // terminate if we timed out
							WaitForSingleObject(other_winamp_process, 1000); // wait some more because TerminateProcess() returns immediately
						}
						CloseHandle(other_winamp_process); // release our reference to the handle
					}
				}
			}
			RemoveRegistrar();
			ExitProcess(0);
		}
		else if (!_wcsnicmp(lpszCmdParam, L"/RETURN=", 8))
		{
			wchar_t p[40] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 8);
			GetParameter(lpszCmdParam, p, 40);
			ExitProcess(_wtoi(p));
		}
#ifndef _WIN64
		else if (!_wcsnicmp(lpszCmdParam, L"/BURN=", 6))
		{
			wchar_t p[1024] = {0};
			unsigned int retCode;
			lpszCmdParam = SkipXW(lpszCmdParam, 6);
			GetParameter(lpszCmdParam, p, 1024);
			CoInitialize(0); 
			InitCommonControls();
			setup_config();
			Wasabi_Load();
			SpectralAnalyzer_Create();
			w5s_init();
			in_init();
			config_read(1);
			retCode = burn_doBurn(AutoChar(p), hMainWindow, hMainInstance);
			in_deinit();
			w5s_deinit();
			Wasabi_Unload();
			SpectralAnalyzer_Destroy();
			RemoveRegistrar();
			ExitProcess(retCode);
		}
#endif
		else if (!_wcsnicmp(lpszCmdParam, L"/WATCHER", 8))
		{
			wchar_t p[2048] = {0};
			lpszCmdParam = SkipXW(lpszCmdParam, 9);
			GetParameter(lpszCmdParam, p, 2048);
			// eat parameter for now...
			RemoveRegistrar();
			ExitProcess(0); // and do not do anything...
		}
		else if (*lpszCmdParam == L'/') // ignore /options :)
		{
			lpszCmdParam = SkipXW(lpszCmdParam, 1);
		}
		else
			break;

		lpszCmdParam = FindNextCommand(lpszCmdParam);
	}

	return lpszCmdParam;
}

void parseCmdLine(wchar_t *cmdline, HWND hwnd)
{
	wchar_t buf[MAX_PATH*4];
	wchar_t tmp[MAX_PATH];
	wchar_t *p;
	if (wcsstr(cmdline, L"/BOOKMARK") == cmdline)
	{
		wchar_t bookmark[1024];
		cmdline = SkipXW(cmdline, 9);
		GetParameter(cmdline, bookmark, 1024);

		COPYDATASTRUCT cds;
		cds.dwData = IPC_ADDBOOKMARKW;
		cds.lpData = (void *) bookmark;
		cds.cbData = sizeof(wchar_t)*(lstrlenW((wchar_t*) cds.lpData) + 1);
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
		return ;
	}
	else if (wcsstr(cmdline, L"/HANDLE") == cmdline)
	{
		wchar_t uri[1024];
		cmdline = SkipXW(cmdline, 7);
		GetParameter(cmdline, uri, 1024);
		COPYDATASTRUCT cds;
		cds.dwData = IPC_HANDLE_URI;
		cds.lpData = (void *) uri;
		cds.cbData = sizeof(wchar_t)*(lstrlenW((wchar_t*) cds.lpData) + 1);
		SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
		return ;
	}
	lstrcpynW(buf, cmdline, MAX_PATH*4);
	p = buf;

	wchar_t param[1024] = {0};
	while (*p)
	{
		p = EatSpaces(p);

		GetParameter(p, param, 1024);
		if (!hwnd)
		{
			PlayList_appendthing(param, 0, 0);
		}
		else
		{
			COPYDATASTRUCT cds;
			wchar_t *p;
			if (!PathIsURLW(param) && GetFullPathNameW(param, MAX_PATH, tmp, &p) && tmp[0])
			{
				cds.dwData = IPC_PLAYFILEW;
				cds.lpData = (void *) tmp;
				cds.cbData = sizeof(wchar_t)*(lstrlenW((wchar_t *) cds.lpData) + 1);
				SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
			}
			else
			{
				cds.dwData = IPC_PLAYFILEW;
				cds.lpData = (void *) param;
				cds.cbData = sizeof(wchar_t) * (lstrlenW((wchar_t *) cds.lpData) + 1);
				SendMessage(hwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
			}
		}
		p = FindNextCommand(p);
	}
} // parseCmdLine()

wchar_t *CheckFileBase(wchar_t *lpszCmdParam, HWND hwnd_other, int *exit, int mode)
{ 
	wchar_t buf[32];
	*exit=0;
	lstrcpynW(buf, extensionW(lpszCmdParam), 32);
	if (wcsstr(buf, L"\"")) wcsstr(buf, L"\"")[0] = 0;

	// process .wsz/.wal file or .wlz (depending on the mode enabled)
	if ((!mode && (!_wcsicmp(buf, L"wsz") || !_wcsicmp(buf, L"wal"))) || (mode && !_wcsicmp(buf, L"wlz")))
	{
		wchar_t *p = lpszCmdParam, buf[MAX_PATH] = {0}, buf2[MAX_PATH] = {0},
					 outname[MAX_PATH] = {0}, current[MAX_PATH] = {0};

		while (*p == L' ') p++;
		if (*p == L'\"') { p++; if (wcsstr(p, L"\"")) wcsstr(p, L"\"")[0] = 0; }

		// this is roughly equivalent to PathUndecorate, which we can't use because it requires IE 5.0+
		StringCchCopyW(outname, MAX_PATH, PathFindFileNameW(p));
		StringCchCopyW(buf, MAX_PATH, (!mode ? SKINDIR : LANGDIR));
		PathCombineW(buf2, buf, outname);

		void _r_sW(const char *name, wchar_t *data, int mlen);
		_r_sW((!mode?"skin":"langpack"), current, MAX_PATH);

		bool name_match = !_wcsicmp(outname, current);
		bool file_match = !_wcsicmp(p, buf2);
		//if (_wcsicmp(outname, current))
		if (!name_match || name_match && !file_match)
		{
			if (LPMessageBox(NULL, !mode?IDS_SKINS_INSTALL_PROMPT:IDS_LANG_INSTALL_PROMPT,
				!mode?IDS_SKINS_INSTALL_HEADER:IDS_LANG_INSTALL_HEADER,
				MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				*exit = 1;
				return lpszCmdParam;
			}
			else
			{
				if(*exit == -2)
				{
					*exit = -1;
				}
			}

			{
				wchar_t *tmp;
				tmp = outname + lstrlenW(outname);
				size_t tmpsize = MAX_PATH - (tmp - outname);
				while (tmp >= outname && *tmp != L'[') tmp--;
				if(!mode)
				{
					if (tmp >= outname && tmp[1] && !_wcsicmp(tmp + 2, L"].wsz")) 
						StringCchCopyW(tmp, tmpsize, L".wsz");
					if (tmp >= outname && tmp[1] && !_wcsicmp(tmp + 2, L"].wal")) 
						StringCchCopyW(tmp, tmpsize, L".wal");
				}
				else
				{
					if (tmp >= outname && tmp[1] && !_wcsicmp(tmp + 2, L"].wlz")) 
						StringCchCopyW(tmp, tmpsize, L".wlz");
				}
			}

			IFileTypeRegistrar *registrar=0;
			if (GetRegistrar(&registrar) == 0 && registrar)
			{
				if (FAILED(registrar->InstallItem(p, buf, outname)))
				{
					wchar_t buffer[MAX_PATH*3];
					StringCchPrintfW(buffer,sizeof(buffer),getStringW((!mode?IDS_SKINS_INSTALL_ERROR:IDS_LANG_INSTALL_ERROR),NULL,0),outname,p,buf);
					MessageBoxW(NULL, buffer, getStringW(!mode?IDS_SKINS_INSTALL_HEADER:IDS_LANG_INSTALL_HEADER,NULL,0), MB_OK | MB_ICONEXCLAMATION);
				}
				registrar->Release();
			}
		}

		if (hwnd_other)
		{
			if(!mode)
			{
				_w_sW("skin", outname);
				COPYDATASTRUCT cds;
				cds.dwData = IPC_SETSKINW;
				cds.lpData = (void *) outname;
				cds.cbData = sizeof(wchar_t)*(lstrlenW(outname) + 1);
				SendMessage(hwnd_other, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
				ShowWindow(hwnd_other, SW_RESTORE);
				SetForegroundWindow(hwnd_other);
			}
			else
			{
				// since we can't reliably unload resources on the fly, force a restart
				_w_sW("langpack", outname);
				PostMessage(hwnd_other,WM_USER,0,IPC_RESTARTWINAMP);
			}
			*exit=1;
		}
		else
		{
			if(!mode)
			{
				g_skinloadedmanually = 1;
				_w_sW("skin", outname);
			}
			else
			{
				_w_sW("langpack", outname);
			}
		}
		lpszCmdParam = L"";
	}

	return lpszCmdParam;
}