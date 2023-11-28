#include "main.h"
#include "loadini.h"
#include "AutoWide.h"
#include "../Winamp/wa_ipc.h"
wchar_t INI_FILE[MAX_PATH]=L"";
void IniFile(HWND hMainWindow)
{
	if (!INI_FILE[0])
	{
		char *p = (char *)SendMessage(hMainWindow, WM_WA_IPC, 0, IPC_GETINIFILE);
		lstrcpyn(INI_FILE, AutoWide(p), MAX_PATH);
	}
}
