#ifndef NULLSOFT_WINAMP_PLAYLIST_H
#define NULLSOFT_WINAMP_PLAYLIST_H

#include "../playlist/api_playlistloadercallback.h"
class Playlist : public api_playlistloadercallback
{
public:
  void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info); 
};

#endif