#ifndef NULLOSFT_DROPBOX_DOCUMENT_CALLBACK_HEADER
#define NULLOSFT_DROPBOX_DOCUMENT_CALLBACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/syscb/callbacks/syscb.h>
#include "../playlist/api_playlists.h"

class Document;
class api_playlists;

class DocumentCallback: public SysCallback
{
public:
	DocumentCallback(Document *pDocument);
	~DocumentCallback();

public:
	FOURCC GetEventType() { return api_playlists::SYSCALLBACK; }
	int Notify(int msg, intptr_t param1 = 0, intptr_t param2 = 0);
	int Register();
	int Unregister();
protected:
	BOOL IsMineDocument(api_playlists *playlistsApi, size_t index);
protected:
	RECVS_DISPATCH;
protected:
	Document *pDoc;
};

#endif // NULLOSFT_DROPBOX_DOCUMENT_CALLBACK_HEADER