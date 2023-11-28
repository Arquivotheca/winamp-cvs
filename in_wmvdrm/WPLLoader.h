#ifndef NULLSOFT_PLAYLIST_WPL_LOADER_H
#define NULLSOFT_PLAYLIST_WPL_LOADER_H

#include "../playlist/api_playlistloader.h"
#include "../playlist/api_playlistloadercallback.h"
#include <stdio.h>
class WPLLoader : public api_playlistloader
{
public:
	int Load(const wchar_t *filename, api_playlistloadercallback *playlist);

public:
	WPLLoader();
	virtual ~WPLLoader(void);

protected:
	RECVS_DISPATCH;
};
#endif