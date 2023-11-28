#include "./main.h"
#include "./feMlPlaylist.h"
#include "./fiPlaylist.h"

static void CopyPlaylistData(mlPlaylist *dest, const mlPlaylist *src)
{
	dest->filename = lfh_strdup(src->filename);
	dest->title = lfh_strdup(src->title);

	dest->length = src->length;
	dest->numItems = src->numItems;
	
}

static void ReleasePlaylistData(mlPlaylist *playlist)
{
	if(NULL != playlist->filename)
		lfh_free((void*)playlist->filename);
	if(NULL != playlist->title)
		lfh_free((void*)playlist->title);
}

MlPlaylistEnumerator::MlPlaylistEnumerator(const mlPlaylist **pMLPlaylists, ULONG playlistCount)
	: ref(1), playlists(NULL), count(playlistCount), cursor(0)
{
	playlists = (mlPlaylist*)malloc(sizeof(mlPlaylist) * count);
	for (ULONG i = 0; i < count; i++)
	{
		CopyPlaylistData(&playlists[i], pMLPlaylists[i]);
	}

}

MlPlaylistEnumerator::~MlPlaylistEnumerator()
{
	for (ULONG i = 0; i < count; i++)
	{
		ReleasePlaylistData(&playlists[i]);
	}
	free(playlists);
}

STDMETHODIMP_(ULONG) MlPlaylistEnumerator::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) MlPlaylistEnumerator::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP MlPlaylistEnumerator::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IFileEnumerator))
		*ppvObject = (IFileEnumerator*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}


STDMETHODIMP MlPlaylistEnumerator::Next(ULONG celt, IFileInfo **pfiBuffer, ULONG *pceltFetched)
{
	ULONG cReturn = 0L;
	if(celt <= 0 || pfiBuffer == NULL || NULL == playlists || cursor >= count)
		return S_FALSE;

	if(pceltFetched == NULL && celt != 1)
		return S_FALSE;

	if(pceltFetched != NULL)
		*pceltFetched = 0;
	
	HRESULT hr = S_OK;
	while(celt > 0 && cursor < count)
	{
		PlaylistFileInfo::CreateInstance(playlists[cursor].filename, NULL, &pfiBuffer[cReturn]);
		cReturn++;
		celt--;
		cursor++;
	}
	
	if (pceltFetched) 
		*pceltFetched = (cReturn - celt);

	return S_OK;
}

STDMETHODIMP MlPlaylistEnumerator::Skip(ULONG celt)
{
	if((cursor + celt) >= count)
		return S_FALSE;

	cursor += celt;
	return S_OK;
}

STDMETHODIMP MlPlaylistEnumerator::Reset(void)
{
	cursor = 0;
	return S_OK;
}