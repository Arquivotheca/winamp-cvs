#ifndef NULLSOFT_IN_WMVDRM_WMPLAYLIST_H
#define NULLSOFT_IN_WMVDRM_WMPLAYLIST_H

#include "../playlist/ifc_playlistloadercallback.h"

class WMPlaylist : public ifc_playlistloadercallback
{
public:
	WMPlaylist() : playstring(0), playlistFilename(0)
	{
	}
	~WMPlaylist()
	{
		free(playstring);
		free(playlistFilename);
	}
	void Clear()
	{
			free(playstring);
		free(playlistFilename);
		playstring=0;
		playlistFilename=0;
	}
  void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info); 

  const wchar_t *GetFileName();
	const wchar_t *GetOriginalFileName();
  /* TODO: need something like these, just not sure exact what yet
  bool ForceStartTime(int &);
  bool ForceLength(int &);
  bool ForceNoSeek();
  */
	bool IsMe(const char *filename);
	bool IsMe(const wchar_t *filename);
protected:
  RECVS_DISPATCH;

public:
	wchar_t *playstring;
	wchar_t *playlistFilename;
};

extern WMPlaylist activePlaylist;
#endif