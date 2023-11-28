#pragma once
#include "../winamp/in2.h"
extern In_Module plugin;
extern HANDLE killswitch;
extern int g_duration;
DWORD CALLBACK OggPlayThread(LPVOID param);

// INT_PTR CALLBACK InfoDialog(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
