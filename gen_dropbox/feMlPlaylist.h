#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_MLPLAYLIST_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_MLPLAYLIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"
#include "../gen_ml/ml.h"


class MlPlaylistEnumerator : public IFileEnumerator
{
public:
	MlPlaylistEnumerator(const mlPlaylist **pMLPlaylists, ULONG playlistCount);
	~MlPlaylistEnumerator();

public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID, PVOID *);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG, IFileInfo **, ULONG *);
	STDMETHOD(Skip)(ULONG);
	STDMETHOD(Reset)(void);

protected:
	ULONG ref;
	mlPlaylist *playlists;
	ULONG count;
	ULONG cursor;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_MLPLAYLIST_HEADER