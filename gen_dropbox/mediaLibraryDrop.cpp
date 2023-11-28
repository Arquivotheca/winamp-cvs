#include "./main.h"
#include "./mediaLibraryDrop.h"
#include "./fileEnumInterface.h"
#include "./feItemRecordList.h"
#include "./feFileNames.h"
#include "./feMlPlaylist.h"
#include "./dropWindow.h"
#include "./document.h"


MlDropItemProcessor::MlDropItemProcessor(mlDropItemStruct *pDropItem, HWND hwndDropBox) 
	: ref(1), hDropBox(hwndDropBox), pdis(pDropItem)
{
}

MlDropItemProcessor::~MlDropItemProcessor()
{
}

BOOL MlDropItemProcessor::CanProcess(mlDropItemStruct *pDropItem)
{
	if (NULL == pDropItem)
		return FALSE;

	switch(pDropItem->type)
	{
		case ML_TYPE_ITEMRECORDLIST:
		case ML_TYPE_ITEMRECORDLISTW:
		case ML_TYPE_FILENAMES:
		case ML_TYPE_FILENAMESW:
		case ML_TYPE_STREAMNAMES:
		case ML_TYPE_STREAMNAMESW:
		case ML_TYPE_CDTRACKS:
		case ML_TYPE_PLAYLIST:
		case ML_TYPE_PLAYLISTS:
			return TRUE;
	}
	return FALSE;
}

STDMETHODIMP_(ULONG) MlDropItemProcessor::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) MlDropItemProcessor::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);

	return r;
}

STDMETHODIMP MlDropItemProcessor::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (NULL == ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IClipboardFormatProcessor))
		*ppvObject = (IClipboardFormatProcessor*)this;
	else if (IsEqualIID(riid, IID_IUnknown))
		*ppvObject = (IUnknown*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((IUnknown*)*ppvObject)->AddRef();
    return S_OK;
}

void MlDropItemProcessor::SetDropItem(mlDropItemStruct *pDropItem)
{
	pdis = pDropItem;
}

HRESULT MlDropItemProcessor::GetFileEnumerator(mlDropItemStruct *pDropItem, IFileEnumerator **ppfe)
{
	if (NULL == ppfe)
		return E_POINTER;

	*ppfe = NULL;

	if (!CanProcess(pDropItem))
		return E_NOTIMPL;

	switch(pDropItem->type)
	{
		case ML_TYPE_ITEMRECORDLIST: 
		case ML_TYPE_ITEMRECORDLISTW:
			*ppfe = (IFileEnumerator*)new ItemRecordListEnumerator((itemRecordList*)pDropItem->data, (ML_TYPE_ITEMRECORDLISTW == pDropItem->type));
			break;
		case ML_TYPE_CDTRACKS:
			{
				METAKEY szFilter[] = {METAKEY_TRACKTITLE, METAKEY_TRACKLENGTH};
				ItemRecordListEnumerator *itemEnum = new ItemRecordListEnumerator((itemRecordList*)pDropItem->data, FALSE);
				itemEnum->SetKeyFilter(szFilter, ARRAYSIZE(szFilter));
				*ppfe = (IFileEnumerator*)itemEnum;
			}
			break;
		case ML_TYPE_FILENAMES:
		case ML_TYPE_STREAMNAMES:
			*ppfe = (IFileEnumerator*)new FileNamesEnumeratorA((LPCSTR)pDropItem->data);
			break;
		case ML_TYPE_FILENAMESW:
		case ML_TYPE_STREAMNAMESW:
			*ppfe = (IFileEnumerator*)new FileNamesEnumeratorW((LPCWSTR)pDropItem->data);
			break;
		case ML_TYPE_PLAYLIST:
			{
				const mlPlaylist *playlist = (mlPlaylist*)pDropItem->data;
				*ppfe = (IFileEnumerator*)new MlPlaylistEnumerator(&playlist, 1);
			}
			break;
		case ML_TYPE_PLAYLISTS:
			{
				INT count = 0;
				const mlPlaylist **playlists = (const mlPlaylist**)pDropItem->data;
				if (NULL != playlists)
					for (; NULL != playlists[count]; count++);
				
				if (count > 0)
					*ppfe = (IFileEnumerator*)new MlPlaylistEnumerator(playlists, count);
			}
			break;
		default:
			return E_NOTIMPL;
	}
	
	if(NULL == *ppfe)
		return E_OUTOFMEMORY;

	return S_OK;
}

STDMETHODIMP MlDropItemProcessor::Process(INT iInsert)
{	if (NULL == pdis || NULL == hDropBox)
		return E_POINTER;
	
	if (!CanProcess(pdis))
		return E_NOTIMPL;
	
	HRESULT hr;
	IFileEnumerator *enumerator;
	
	hr = GetFileEnumerator(pdis, &enumerator);
	if (FAILED(hr))
		return hr;
	
	if (!DropboxWindow_InsertEnumerator(hDropBox, iInsert, enumerator))
			hr = E_FAIL;
		enumerator->Release();

	return S_OK;
}
