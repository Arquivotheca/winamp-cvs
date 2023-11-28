#ifndef NULLSOFT_PODCAST_PLUGIN_CHANNEL_EDITOR_HEADER
#define NULLSOFT_PODCAST_PLUGIN_CHANNEL_EDITOR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#define CEF_CREATENEW		0x00000001
#define CEF_CENTEROWNER		0x00000002

INT_PTR ChannelEditor_Show(HWND hOwner, size_t channelIndex, UINT flags);

#endif //NULLSOFT_PODCAST_PLUGIN_CHANNEL_EDITOR_HEADER