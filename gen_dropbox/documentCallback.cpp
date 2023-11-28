#include "./main.h"
#include "./DocumentCallback.h"
#include "./wasabiApi.h"
#include "./Document.h"
#include "../playlist/api_playlists.h"

DocumentCallback::DocumentCallback(Document *pDocument) : pDoc(pDocument)
{
}

DocumentCallback::~DocumentCallback()
{
	Unregister();
}

int DocumentCallback::Register()
{
	return WASABI_API_SYSCB->syscb_registerCallback(this);
}

int DocumentCallback::Unregister()
{
	return WASABI_API_SYSCB->syscb_deregisterCallback(this);
}


BOOL DocumentCallback::IsMineDocument(api_playlists *playlistsApi, size_t index)
{	
	if (NULL == pDoc || NULL == playlistsApi)
		return FALSE;

	BOOL mineDoc = FALSE;
	
	LPCWSTR fileName = playlistsApi->GetFilename(index);
	if (NULL != fileName || L'\0' != fileName)
	{
		TCHAR szPath[MAX_PATH];
		mineDoc =(SUCCEEDED(pDoc->GetPath(szPath, ARRAYSIZE(szPath))) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, fileName, -1, szPath, -1));

		TRACE_LINE(szPath);

	}
	return mineDoc;
}
int DocumentCallback::Notify(int msg, intptr_t param1, intptr_t param2)
{
	if (param2 == GetPluginUID()) 
		return 0;

	api_playlists *playlistsApi;
	

	switch (msg)
	{
		case api_playlists::PLAYLIST_SAVED:
			playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
			if (NULL != playlistsApi)
			{
				playlistsApi->Lock();
				BOOL checkRequired = IsMineDocument(playlistsApi, param1);
				playlistsApi->Unlock();
				ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);		
				
				if (checkRequired)
					pDoc->Notify(Document::EventCheckFileModified, 0);
			}
			break;
		case api_playlists::PLAYLIST_RENAMED:
			playlistsApi = QueryWasabiInterface(api_playlists, api_playlistsGUID);
			if (NULL != playlistsApi)
			{
				playlistsApi->Lock();
				if (IsMineDocument(playlistsApi, param1))
					pDoc->SetTitle(playlistsApi->GetName(param1));
				
				playlistsApi->Unlock();
				ReleaseWasabiInterface(api_playlistsGUID, playlistsApi);
			}
			
			break;
	}

	
	return 0;
}

#define CBCLASS DocumentCallback
START_DISPATCH;
CB(SYSCALLBACK_GETEVENTTYPE, GetEventType);
CB(SYSCALLBACK_NOTIFY, Notify);
END_DISPATCH;
#undef CBCLASS