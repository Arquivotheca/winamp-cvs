#ifndef NULLSOFT_DROPBOX_PLUGIN_PLAYLISTMETAREADER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_PLAYLISTMETAREADER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./fileMetaInterface.h"
#include "./genericItemMeta.h"
#include "../playlist/ifc_playlistloadercallback.h"

class PlaylistMetaReader : public IFileMetaReader, public ifc_playlistloadercallback
{
public:
	typedef enum
	{
		READ_TITLE = 0x0001,
		READ_LENGTH = 0x0002,
		READ_COUNT = 0x0004,
	} READFLAGS;
public:
	PlaylistMetaReader(GenericItemMeta *pMetaObject, INT readMode, UINT readFlags);
	virtual ~PlaylistMetaReader();

public:
	static BOOL CanRead(METAKEY metaKey);
public:

	/*** IUnknown ***/
	STDMETHOD(QueryInterface)(REFIID riid, PVOID *ppvObject);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	/*** IFileMetaReader ***/
	STDMETHOD(Read)(void);
	STDMETHOD(SetCookie)(DWORD cookie);
	STDMETHOD(GetCookie)(DWORD *pCookie);

	/*** ifc_playlistloadercallback ***/
	int OnFile(const wchar_t *filename, const wchar_t *title, int lengthInMS, ifc_plentryinfo *info);
	int OnPlaylistInfo(const wchar_t *playlistName, size_t numEntries, ifc_plentryinfo *info);
	const wchar_t *GetBasePath();

protected:
	BOOL TitleFromPlaylists(LPCTSTR pszPlaylist);
	BOOL TitleFromFileName(LPCTSTR pszPlaylist);
	void SetTitle(LPCTSTR pszName);
	RECVS_DISPATCH;

protected:
	ULONG ref;
	INT mode;
	UINT flags;
	GenericItemMeta *pObject;
	LPTSTR	pszTitle;
	size_t	counter;
	int		length;
	DWORD	cookie;
	
};

#endif //NULLSOFT_DROPBOX_PLUGIN_PLAYLISTMETAREADER_HEADER