#ifndef NULLOSFT_DROPBOX_PLUGIN_HEADERCALLBACKS_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_HEADERCALLBACKS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class Meterbar;
class Toolbar;
class PlaylistDropTarget;

BOOL DropboxHeader_RegisterMeterbarCallback(HWND hwnd, Meterbar *instance);
BOOL DropboxHeader_RegisterToolbarCallback(HWND hwnd, Toolbar *instance);
BOOL DropboxHeader_RegisterPlDropCallback(HWND hwnd, PlaylistDropTarget *instance);


#endif // NULLOSFT_DROPBOX_PLUGIN_HEADERCALLBACKS_HEADER