#ifndef NULLOSFT_DROPBOX_PLUGIN_SHORTCUTS_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_SHORTCUTS_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

BOOL PluginShortcut_Register(const UUID &classUid, const ACCEL *pAccel);
BOOL PluginShortcut_Unregister(const UUID &classUid);

#endif //NULLOSFT_DROPBOX_PLUGIN_SHORTCUTS_HEADER
