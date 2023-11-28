#ifndef NULLSOFT_ML_PLAYLISTS_PLAYLIST_H
#define NULLSOFT_ML_PLAYLISTS_PLAYLIST_H

#include "../playlist/api_playlist.h"
#include "../nu/Vector.h"
#include "../nu/PtrList.h"
#include <windows.h> // for MAX_PATH
#include <bfc/multipatch.h>
#include "../playlist/pl_entry.h"
#include "../playlist/api_playlistloadercallback.h"

enum {	patch_playlist,	patch_playlistloadercallback};

class Playlist : public MultiPatch<patch_playlist, ifc_playlist>,
	public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	Playlist() : lengthInMS(0) {}
	~Playlist();
	void Clear();
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);
	void AppendWithInfo(const wchar_t *filename, const wchar_t *title, int lengthInMS);
	size_t GetNumItems();

	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	const wchar_t *ItemTitle(size_t item);
	const wchar_t *ItemName(size_t item);
	int GetItemLengthMilliseconds(size_t item); // TODO: maybe microsecond for better resolution?
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);

	bool IsCached(size_t item);
	void ClearCache(size_t item);

	void SetItemFilename(size_t item, const wchar_t *filename);
	void SetItemTitle(size_t item, const wchar_t *title);
	void SetItemLengthMilliseconds(size_t item, int length);

	int Reverse();
	int Swap(size_t item1, size_t item2);
	int Randomize(int (*generator)());
	void Remove(size_t item);

	int SortByTitle();
	int SortByFilename();
	int SortByDirectory(); //sorts by directory and then by filename

	void InsertPlaylist(Playlist &copy, size_t index);
	void AppendPlaylist(Playlist &copy);

protected:
	RECVS_MULTIPATCH;

public:
	typedef nu::PtrList<pl_entry> PlaylistEntries;
	PlaylistEntries entries;
	uint64_t lengthInMS;
};
#endif