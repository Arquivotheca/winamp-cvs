#ifndef NULLSOFT_PLAYLIST_M3U_LOADER_H
#define NULLSOFT_PLAYLIST_M3U_LOADER_H

#include "ifc_playlistloader.h"
#include "ifc_playlistloadercallback.h"
#include <stdio.h>

class M3ULoader : public ifc_playlistloader
{
public:
	int Load(const wchar_t *filename, ifc_playlistloadercallback *playlist);
	int OnFileHelper(ifc_playlistloadercallback *playlist, const char *trackName, const wchar_t *title, int length, const wchar_t *rootPath, ifc_plentryinfo *extraInfo);

public:
	M3ULoader();
	virtual ~M3ULoader(void);

protected:
	RECVS_DISPATCH;
	bool utf8;
	wchar_t wideFilename[1040];
	wchar_t wideTitle[400];
};
#endif