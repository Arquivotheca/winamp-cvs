#ifndef NULLSOFT_DROPBOX_PLUGIN_FRAMECACHE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_FRAMECACHE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class FrameCache
{
public:
	typedef enum
	{
		ItemOutOfBounds = -1,
		ItemStateClear = 0,
		ItemStateFetching = 1,
		ItemStateFetched = 2,
		ItemStateCached = 3,
	} ItemState;

	typedef BOOL (CALLBACK *EnumProc)(INT /*first*/, INT /*last*/, ULONG_PTR /*param*/);
	typedef void (CALLBACK *ModifiedCallback)(BOOL /*cacheModified*/, ULONG_PTR /*param*/);
	
public:
	FrameCache();
	~FrameCache();

public:
	void Reset();
	void SetSize(INT newSize, BOOL bPreserve);
	INT GetSize();
	void SetTop(INT newTop, BOOL bPreserve);

	INT GetItemState(INT itemIndex);
	BOOL SetItemState(INT itemIndex, INT itemState, BOOL markModified);
	BOOL SetItemStateEx(INT first, INT last, INT itemState, BOOL markModified);

	BOOL GetModified();
	void SetModified(BOOL bModified);
	void RegisterModifiedCallback(ModifiedCallback callback, ULONG_PTR param);

	BOOL Enumerate(INT itemState, FrameCache::EnumProc callback, ULONG_PTR param);
	BOOL EnumerateEx(INT first, INT last, INT itemState, FrameCache::EnumProc callback, ULONG_PTR param);

private:
	CRITICAL_SECTION lockCache;
	
	INT top;
	INT size;
	BYTE *buffer;
	BOOL modified;
	
	ULONG_PTR modifiedParam;
	ModifiedCallback modifiedCallback;

};


#endif // NULLSOFT_DROPBOX_PLUGIN_FRAMECACHE_HEADER