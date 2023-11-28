#pragma once
#include "../playlist/api_playlist.h"
#include "../nu/Vector.h"
#include "../nu/PtrList.h"
#include <windows.h> // for MAX_PATH
#include <bfc/multipatch.h>
#include "../playlist/api_playlistloadercallback.h"

class merge_pl_entry
{
public:
	size_t GetFilename(wchar_t *filename, size_t filenameCch);
	size_t GetTitle(wchar_t *title, size_t titleCch);
	int GetLengthInMilliseconds();
	size_t GetExtendedInfo(const wchar_t *metadata, wchar_t *info, size_t infoCch);

	void SetFilename(const wchar_t *filename);
	void SetTitle(const wchar_t *title);
	void SetLengthMilliseconds(int length);
public:
	merge_pl_entry() 
	{
		filename[0]=0;
		filetitle[0]=0;
		length=-1;
		cached=false;
	}
	~merge_pl_entry();
	merge_pl_entry(const wchar_t *fn, const wchar_t *ft, int len);
//protected:
  wchar_t *filename;
  wchar_t *filetitle;
  int length;
	bool cached;
};

enum {	patch_playlist,	patch_playlistloadercallback};

class MergePlaylist : public MultiPatch<patch_playlist, ifc_playlist>,
	public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
{
public:
	MergePlaylist();
	~MergePlaylist();
	void Clear();
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS,ifc_plentryinfo *info);
	void AppendWithInfo(const wchar_t *filename, const wchar_t *title, int lengthInMS);
	//void Append(const wchar_t *filename);
	size_t GetNumItems();
	
	size_t GetItem(size_t item, wchar_t *filename, size_t filenameCch);
	size_t GetItemTitle(size_t item, wchar_t *title, size_t titleCch);
	const wchar_t *ItemTitle(size_t item);
	const wchar_t *ItemName(size_t item);
	int GetItemLengthMilliseconds(size_t item); 
	size_t GetItemExtendedInfo(size_t item, const wchar_t *metadata, wchar_t *info, size_t infoCch);

	void SetItemFilename(size_t item, const wchar_t *filename);
	void SetItemTitle(size_t item, const wchar_t *title);
	void SetItemLengthMilliseconds(size_t item, int length);

	void AppendPlaylist(MergePlaylist &copy);

	bool HasFilename(const wchar_t *filename);
	
	uint64_t total_time;
protected:
	RECVS_MULTIPATCH;

public:
//private:
	typedef nu::PtrList<merge_pl_entry> PlaylistEntries;
	PlaylistEntries entries;
};
