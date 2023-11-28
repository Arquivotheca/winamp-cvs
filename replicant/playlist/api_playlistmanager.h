#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "service/types.h"
#include "nx/nxuri.h"
class cb_playlistloader;
class ifc_playlist;

// {C5618774-7177-43aa-9906-933C9F40EBDC}
static const GUID api_playlistmanagerGUID =
  {
    0xc5618774, 0x7177, 0x43aa, { 0x99, 0x6, 0x93, 0x3c, 0x9f, 0x40, 0xeb, 0xdc }
  };

class api_playlistmanager : public Wasabi2::Dispatchable
{
protected:
	api_playlistmanager() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION)	{}
	~api_playlistmanager()	{}
public:
	static GUID GetServiceType() { return SVC_TYPE_UNIQUE; }
	static GUID GetServiceGUID() { return api_playlistmanagerGUID; }
//	int Load(nx_uri_t filename, cb_playlistloader *playlist) { return PlaylistManager_LoadAs(filename, filename, playlist); }
	/* loads the playlist as a file but uses a separate filename to hint the playlist type (by extension), pass, e.g. ".pls" */
	int LoadAs(nx_uri_t filename, nx_string_t ext, cb_playlistloader *playlist) { return PlaylistManager_LoadAs(filename, ext, playlist); }
#if 0

	int LoadFromDialog(const wchar_t *fns, ifc_playlistloadercallback *playlist);
	int LoadFromANSIDialog(const char *fns, ifc_playlistloadercallback *playlist);
	int Save(const wchar_t *filename, ifc_playlist *playlist);
	size_t Copy(const wchar_t *destFn, const wchar_t *srcFn); // returns number of items copied
	size_t CountItems(const wchar_t *filename);
	int GetLengthMilliseconds(const wchar_t *filename);
	void Randomize(ifc_playlist *playlist);
	void Reverse(ifc_playlist *playlist);
	void LoadDirectory(const wchar_t *directory, ifc_playlistloadercallback *callback, ifc_playlistdirectorycallback *dirCallback);
	bool CanLoad(const wchar_t *filename);
	void GetExtensionList(wchar_t *extensionList, size_t extensionListCch);
	void GetFilterList(wchar_t *extensionList, size_t extensionListCch);
	const wchar_t *EnumExtension(size_t num);
#endif
private:
	virtual int WASABICALL PlaylistManager_LoadAs(nx_uri_t filename, nx_string_t ext, cb_playlistloader *playlist)=0;
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};
