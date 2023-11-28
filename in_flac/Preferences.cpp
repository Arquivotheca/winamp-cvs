/*
** Copyright (C) 2007-2011 Nullsoft, Inc.
**
** This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held 
** liable for any damages arising from the use of this software. 
**
** Permission is granted to anyone to use this software for any purpose, including commercial applications, and to 
** alter it and redistribute it freely, subject to the following restrictions:
**
**   1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
**      If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
**
**   2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
**
**   3. This notice may not be removed or altered from any source distribution.
**
** Author: Ben Allison benski@winamp.com
** Created: March 1, 2007
**
*/
#include "main.h"
#include <FLAC/all.h>
#include "resource.h"
#include "../Agave/Language/api_language.h"
#include <strsafe.h>

bool fixBitrate=false;
bool config_average_bitrate=true;
char extensions[256];
void BuildExtensions()
{
	char config_extensions[128];
	GetPrivateProfileString("in_flac", "extensions", DEFAULT_EXTENSIONS, config_extensions, 128, winampINI);

	size_t len=0;
	char *extEnd=0;
	StringCchCopyExA(extensions, 256, config_extensions, &extEnd, &len, 0);
	extEnd++;
	len--;
	StringCchCopyExA(extEnd, len, WASABI_API_LNGSTRING(IDS_FLAC_FILES), &extEnd, &len, 0);
	extEnd++;
	len--;
	*extEnd=0;

	plugin.FileExtensions = extensions;
}

static INT_PTR CALLBACK PreferencesProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			char config_extensions[128];
			GetPrivateProfileString("in_flac", "extensions", DEFAULT_EXTENSIONS, config_extensions, 128, winampINI);
			SetDlgItemTextA(hwndDlg, IDC_EXTENSIONS, config_extensions);
			CheckDlgButton(hwndDlg, IDC_AVERAGE_BITRATE, config_average_bitrate?BST_CHECKED:BST_UNCHECKED);
		}
		return TRUE;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				char config_extensions[128];
				GetDlgItemTextA(hwndDlg, IDC_EXTENSIONS, config_extensions, 128);
				if (!lstrcmpiA(config_extensions, DEFAULT_EXTENSIONS))
					WritePrivateProfileString("in_flac", "extensions", 0, winampINI);
				else
					WritePrivateProfileString("in_flac", "extensions", config_extensions, winampINI);

				BuildExtensions();
				config_average_bitrate = !!IsDlgButtonChecked(hwndDlg, IDC_AVERAGE_BITRATE);
				if (config_average_bitrate)
					WritePrivateProfileString("in_flac", "average_bitrate", "1", winampINI);
				else
					WritePrivateProfileString("in_flac", "average_bitrate", "0", winampINI);

				fixBitrate=true;
				EndDialog(hwndDlg, 0);
			}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
		break;
	}
	return 0;
}

void Config(HWND hwndParent)
{
	WASABI_API_DIALOGBOXW(IDD_PREFERENCES, hwndParent, PreferencesProc);
}