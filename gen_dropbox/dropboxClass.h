#ifndef NULLOSFT_WINAMP_DROPBOX_PLUGIN_DROPBOXWINDOWCLASS_HEADER
#define NULLOSFT_WINAMP_DROPBOX_PLUGIN_DROPBOXWINDOWCLASS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define DBCS_ONEINSTANCE		0x00000001
#define DBCS_SKINWINDOW			0x00000002
#define DBCS_ENABLEBLUR			0x00000004
#define DBCS_WINAMPGROUP			0x00000008
#define DBCS_REMEMBERPROFILE		0x00000010
#define DBCS_DONOTSAVE			0x00000020
#define DBCS_REGISTERPLAYLIST	0x00000040
#define DBCS_SHOWHEADER			0x00000080

typedef struct __DROPBOXCLASSINFO
{
	UUID	classUid;
	ACCEL	shortcut;
	UINT	style;
	LPTSTR  pszTitle; // if (!IS_INTRESOURCE(pszTitle)) - use StrDup()/LocalFree()
	UUID	profileUid;
	INT		x;
	INT		y;
} DROPBOXCLASSINFO;


HRESULT DropboxClass_Save(const DROPBOXCLASSINFO *pdbc);
HRESULT DropboxClass_Load(const UUID &classUid, DROPBOXCLASSINFO *pdbc);
void DropboxClass_FreeString(LPTSTR  pszString);

HRESULT DropboxClass_SaveProfile(const UUID &classUid, const UUID &profileUid);
HRESULT DropboxClass_SavePosition(const UUID &classUid, POINT pt);


#endif //NULLOSFT_WINAMP_DROPBOX_PLUGIN_DROPBOXWINDOWCLASS_HEADER