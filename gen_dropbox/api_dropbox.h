#ifndef NULLOSFT_API_DROPBOX_HEADER
#define NULLOSFT_API_DROPBOX_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include <bfc/dispatch.h>


class api_dropbox : public Dispatchable
{
protected:
	api_dropbox() {}
	~api_dropbox() {}

public:
	HWND CreateDropBox(HWND hParent, const GUID *classUid);

public:
	DISPATCH_CODES
	{
		API_DROPBOX_CREATEDROPBOX = 10,
	};
};

inline HWND api_dropbox::CreateDropBox(HWND hParent, const GUID *classUid)
{
	return _call(API_DROPBOX_CREATEDROPBOX, (HWND)NULL, hParent, classUid);
}

// {D8348906-53FF-44c0-B570-3CD127B5B465}
static const GUID dropBoxApiGuid = 
{ 0xd8348906, 0x53ff, 0x44c0, { 0xb5, 0x70, 0x3c, 0xd1, 0x27, 0xb5, 0xb4, 0x65 } };

#endif // NULLOSFT_API_DROPBOX_HEADER
