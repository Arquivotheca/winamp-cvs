#ifndef NULLOSFT_API_DROPBOX_IMPLEMENTATION_HEADER
#define NULLOSFT_API_DROPBOX_IMPLEMENTATION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./api_dropBox.h"

class DropBoxApi : public api_dropbox
{
public:
	HWND CreateDropBox(HWND hParent, const GUID *classUid);

protected:
	RECVS_DISPATCH;
};

#endif // NULLOSFT_API_DROPBOX_IMPLEMENTATION_HEADER
