#ifndef NULLOSFT_DROPBOX_PLUGIN_EDITBOXTWEAK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_EDITBOXTWEAK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define ETF_NOTIFY_ENTERKEY 	0x00000001 // you will get NM_RETURN (WM_NOTIFY) return 1 if you handled it

BOOL EditboxTweak_Enable(HWND hEdit, UINT tweakFlags);



#endif // NULLOSFT_DROPBOX_PLUGIN_EDITBOXTWEAK_HEADER