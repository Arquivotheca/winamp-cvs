#ifndef NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_PLAYLIST_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_PLAYLIST_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./fileEnumInterface.h"
#include "./fileMetaInterface.h"
#include "../nu/vector.h"
#include "../playlist/ifc_playlistloadercallback.h"

class TypeCollection;

class PlaylistFileEnumerator : public IFileEnumerator, public ifc_playlistloadercallback
{
public:
	PlaylistFileEnumerator(LPCTSTR pszPlaylist);
	~PlaylistFileEnumerator();
	
public:
	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileEnumerator ***/
	STDMETHOD(Next)(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);

	/*** ifc_playlistloadercallback ***/
	void OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);
	void OnPlaylistInfo(const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info);
	const wchar_t *GetBasePath();

protected:
	void ReleaseList();

protected:

	typedef struct 
	{
		LPWSTR pszPath;
		FILEMETARECORD *pMetaRec;
		INT metaCount;
	} PLITEM;

	typedef Vector<PLITEM> ItemList;
	RECVS_DISPATCH;

	ULONG ref;
	LPTSTR pszPath;
	LPTSTR pszFile;
	BOOL bLoaded;
	ItemList list;
	ItemList::iterator cursor;
};


#endif //NULLSOFT_DROPBOX_PLUGIN_FILEENUMERATOR_PLAYLIST_HEADER