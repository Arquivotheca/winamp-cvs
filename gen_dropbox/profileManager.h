#ifndef NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILEMANAGER_HEADER
#define NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILEMANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./profile.h"
#include "../nu/ptrList.h"
#include "../nu/vector.h"

#define PMN_FIRST				(0)
#define PMN_PROFILESELECTED		(PMN_FIRST + 0) // you will get this notification when profile selected

#define PMM_FIRST				(WM_USER + 20)
#define PMM_SETPROFILE			(PMM_FIRST + 0)	// lParam = (LPARAM)(UUID*)pProfileUid; returns TRUE if Ok
#define PMM_GETPROFILE			(PMM_FIRST + 1)	// lParam = 0; returns Profile* of current selection if any. It iws your job to AddRef it
#define PMM_GETIDEALHEIGHT		(PMM_FIRST + 2)	// returns int height
#define PMM_PROFILECHANGED		(PMM_FIRST + 3) // internal message

#define ProfileManagerView_SetProfile(__hwnd, __poiterToProfileUidIn)\
	((BOOL)SNDMSG((__hwnd), PMM_SETPROFILE, 0, (LPARAM)(__poiterToProfileUidIn)))
#define ProfileManagerView_GetProfile(__hwnd)\
	((Profile*)SNDMSG((__hwnd), PMM_GETPROFILE, 0, 0L))
#define ProfileManagerView_GetIdealHeight(__hwnd)\
	((INT)SNDMSG((__hwnd), PMM_GETIDEALHEIGHT, 0, 0L))

class __declspec(novtable) ProfileCallback
{
public:
	typedef enum
	{
		eventNameChanged = 0, // name and/or description
		eventFilterChanged,
		eventViewChanged,  // another view now active
		eventViewConfigChanged, // same view, but view settings changed
		eventProfileCreated,
		eventProfileDeleted,
	} EventCode;

protected:
	ProfileCallback() {};
	~ProfileCallback() {};
public:
	virtual ULONG AddRef() = 0;
	virtual ULONG Release() = 0;
	virtual void Notify(UINT eventId, const UUID *profileUid) = 0;
};

class ProfileManager
{
public:
	ProfileManager();
	~ProfileManager();

public:
	INT RegisterDefault();
	Profile *LoadProfile(const UUID &profileUid);
	INT LoadProfiles(Profile **pszProfiles, INT cchProfilesMax); // returns number of actually loaded profiles
	HWND CreateView(HWND hParent, INT x, INT y, INT cx, INT cy, INT_PTR controlId);
	BOOL RegisterCallback(const UUID &profileUid, ProfileCallback *callback); // register with GUID_NULL to recive notifications for all profiles
	BOOL UnregisterCallback(const UUID &profileUid, ProfileCallback *callback);
	void Notify(const UUID &profileUid, UINT eventId);

protected:
	typedef nu::PtrList<ProfileCallback> CallbackList;
	typedef struct __CallbackEntry
	{
		UUID			profileUid;
		CallbackList	*callbackList;
	} CallbackEntry;

protected:	
	Vector<CallbackEntry> registeredCallbacks;
	CallbackList		  globalCallbacks;
	size_t cachedCallbackIndex;

};

#endif //NULLOSFT_WINAMP_DROPBOX_PLUGIN_PROFILEMANAGER_HEADER
