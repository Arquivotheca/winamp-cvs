#ifndef NULLOSFT_DROPBOX_PLUGIN_WINAMP_HOOK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_WINAMP_HOOK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

typedef HANDLE HWAHOOK;

#define WHCB_OKTOQUIT			0x0001
#define WHCB_RESETFONT			0x0002
#define WHCB_SKINCHANGED			0x0003
#define WHCB_SKINCHANGING		0x0004	
#define WHCB_FILEMETACHANGED		0x0005
#define WHCB_SYSCOLORCHANGE		0x0006

typedef LRESULT (CALLBACK *WHCALLBACK)(HWAHOOK /*hWaHook*/, UINT /*whcbId*/, WPARAM /*param*/, ULONG_PTR /*user*/);  

HWAHOOK AttachWinampHook(HWND hwndWa, WHCALLBACK callback, ULONG_PTR user);
void ReleaseWinampHook(HWND hwndWa, HWAHOOK hWaHook);
LRESULT CallNextWinampHook(HWAHOOK hWaHook, UINT whcbId, WPARAM param);

#endif // NULLOSFT_DROPBOX_PLUGIN_WINAMP_HOOK_HEADER