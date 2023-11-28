#ifndef __WASABI_API_WATCHER_H
#define __WASABI_API_WATCHER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>
#include "./api_watchfilter.h"

// {6E343FAB-96B9-4359-8B03-CE398F26FC8B}
static const GUID watcherGUID = 
{ 0x6E343FAB, 0x96B9, 0x4359, { 0x8B, 0x03, 0xCE, 0x39, 0x8F, 0x26, 0xFC, 0x8B } };

// Filter types
#define WATCHER_FILTERTYPE_NONE			0
#define WATCHER_FILTERTYPE_INCLUDE		1
#define WATCHER_FILTERTYPE_EXCLUDE		2

// File states defenitions reported in MLWATHCERNOTIFY
#define WATCHER_FILESTATE_NOCHANGES		0		// no changes detected (happens if force scan used)
#define WATCHER_FILESTATE_ADDED			1		// file first time detected
#define WATCHER_FILESTATE_REMOVED		2		// file removed ( happens only if trackState enabled)
#define WATCHER_FILESTATE_CHANGED		3		// file changes detected

// Track mode defenittions
#define WATCHER_TRACKMODE_OFF			0		// no state tracking
#define WATCHER_TRACKMODE_INPLACE		1		// tracking files located in the watch folders
#define WATCHER_TRACKMODE_CENTRAL		2		// tracking data located in the central place


#define WATCHER_MSG_STATUS			0		// sent if status changed
#define WATCHER_MSG_FILE			1		// sent if file chaged

class NOVTABLE api_watcher;
// Watch Notification Callback  return codes: 1 - to store file track data, 0 - ignore 
typedef int (*WATCHERNOTIFY)(api_watcher *sender, UINT message, LONG_PTR param, void* userdata);

// Scan watch priorities
#define SCANPRIORITY_IDLE		0
#define SCANPRIORITY_LOWEST		1
#define SCANPRIORITY_LOW		2
#define SCANPRIORITY_NORMAL		3	
#define SCANPRIORITY_HIGH		4	
#define SCANPRIORITY_HIGHEST	5

// Statuses
#define STATUS_ERROR				0x0000

#define STATUS_WATCHER_STOPPED		0x0001
#define STATUS_WATCHER_ACTIVE		0x0002
#define STATUS_WATCHER_STARTING		0x0003
#define STATUS_WATCHER_STOPPING		0x0004

#define STATUS_SCANER_STOPPED		0x0011
#define STATUS_SCANER_ACTIVE		0x0012
#define STATUS_SCANER_FINISHED		0x0013
#define STATUS_SCANER_STOPPING		0x0014
#define STATUS_SCANER_STARTING		0x0015


typedef struct
{
	wchar_t				*file;		// File name.
	unsigned int		cchFile;	// File name length (in characters).
	unsigned int		hFile;		// file name hash code.
	wchar_t				*path;		// Path to the file.
	unsigned int		cchPath;	// Path name length (in characters).
	unsigned int		hPath;		// path hash code (use it:)).
	int					state;		// File state (Can be one of the WATCHER_FILESTATE_*).
	unsigned __int32	sizeLow;	// File size low
    unsigned __int32	sizeHigh;	// File size high
	FILETIME			wTime;		// File write access time.
} WATCHERCHANGEINFO;

class NOVTABLE api_watcher : public Dispatchable
{
protected:
	api_watcher() {}
	~api_watcher() {}
public:
	int Create(const wchar_t *id, const wchar_t *path, int recurse, int trackMode, WATCHERNOTIFY cbNotify);
    void Destroy(void);
	api_watcher* CopyTo(api_watcher *destination);
	int Parse(const void* xml, unsigned int length, const wchar_t* encoding);
	unsigned int GetStringLength(void);
	wchar_t *GetString(wchar_t *buffer, unsigned int bufferLen);

	int Start(void);
	void Stop(void);
	
	void SetExtensionFilterType(int type);
	int SetExtensionFilter(const wchar_t *string, int filterType);
	void SetMinSizeFilter(size_t size);
	void SetTrackingPath(const wchar_t* path);
	WATCHERNOTIFY SetCallBack(WATCHERNOTIFY cbNotify); 
	void* SetUserData(void* data);

	int ForceScan(int reportAll, int priority);
	void StopScan(void);
	
	const wchar_t* GetID(void);
	const wchar_t* GetPath(void);
	int GetRecurse(void);
	int GetTrackingMode(void);
	const wchar_t* GetTrackingPath(void);
	int GetExtensionFilterType(void);
	api_watchfilter* GetExtensionFilter(void);
	size_t GetMinSizeFilter(void);
	int GetStatus(void);

	DISPATCH_CODES
	{
	    API_WATCHER_CREATE			= 0x0010,
	    API_WATCHER_DESTROY			= 0x0020,
		API_WATCHER_COPYTO			= 0x0030,
		API_WATCHER_GETSTRING		= 0x0040,
		API_WATCHER_GETSTRINGLENGTH	= 0x0050,
		API_WATCHER_PARSE			= 0x0060,


		API_WATCHER_START			= 0x0100,
		API_WATCHER_STOP				= 0x0110,
		
		API_WATCHER_SET_EXTFILTERTYPE	= 0x0200,
		API_WATCHER_SET_EXTFILTER		= 0x0210,
		API_WATCHER_SET_SIZEFILTER		= 0x0220,
		API_WATCHER_SET_TRACKPATH		= 0x0230,
		API_WATCHER_SET_USERDATA		= 0x0240,
		API_WATCHER_SET_CALLBACK		= 0x0250,
		
		API_WATCHER_FORCESCAN		= 0x0300,
		API_WATCHER_STOPSCAN			= 0x0310,
		
		API_WATCHER_GET_ID				= 0x0400,
		API_WATCHER_GET_PATH				= 0x0410,
		API_WATCHER_GET_RECURSE			= 0x0420,
		API_WATCHER_GET_TRACKMODE		= 0x0430,
		API_WATCHER_GET_TRACKPATH		= 0x0440,
		API_WATCHER_GET_EXTFILTER		= 0x0450,
		API_WATCHER_GET_EXTFILTERTYPE	= 0x0470,
		API_WATCHER_GET_SIZEFILTER		= 0x0480,
		API_WATCHER_GET_STATUS			= 0x0490,
	};

};

inline int api_watcher::Create(const wchar_t *id, const wchar_t *path, int recurse, int trackMode, WATCHERNOTIFY cbNotify)
{
	return _call(API_WATCHER_CREATE, (int)0, id, path, recurse, trackMode, cbNotify);
}
inline void api_watcher::Destroy(void)
{
	_voidcall(API_WATCHER_DESTROY);
}
inline api_watcher* api_watcher::CopyTo(api_watcher *destination)
{
	return _call(API_WATCHER_COPYTO, (api_watcher*)NULL,  destination);
}
inline int api_watcher::Parse(const void* xml, unsigned int length, const wchar_t* encoding)
{
	return _call(API_WATCHER_PARSE, (int)NULL,  xml, length, encoding);
}
inline unsigned int api_watcher::GetStringLength(void)
{
	return _call(API_WATCHER_GETSTRINGLENGTH, (unsigned int)NULL);
}
inline wchar_t* api_watcher::GetString(wchar_t *buffer, unsigned int bufferLen)
{
	return _call(API_WATCHER_GETSTRING, (wchar_t*)NULL,  buffer, bufferLen);
}
inline int api_watcher::Start(void)
{
	return _call(API_WATCHER_START, (int)0);
}
inline void api_watcher::Stop(void)
{
	_voidcall(API_WATCHER_STOP);
}
inline void api_watcher::SetExtensionFilterType(int type)
{
	_voidcall(API_WATCHER_SET_EXTFILTERTYPE, type);
}
inline int api_watcher::SetExtensionFilter(const wchar_t *string, int filterType)
{
	return _call(API_WATCHER_SET_EXTFILTER, (int)0, string, filterType);
}
inline void api_watcher::SetMinSizeFilter(size_t size)
{
	_voidcall(API_WATCHER_SET_SIZEFILTER, size);
}
inline void api_watcher::SetTrackingPath(const wchar_t* path)
{
	_voidcall(API_WATCHER_SET_TRACKPATH, path);
}
inline WATCHERNOTIFY api_watcher::SetCallBack(WATCHERNOTIFY cbNotify)
{
	return _call(API_WATCHER_SET_CALLBACK, (WATCHERNOTIFY)NULL, cbNotify);
}
inline void* api_watcher::SetUserData(void* data)
{
	return _call(API_WATCHER_SET_USERDATA, (void*)NULL, data);
}

inline int api_watcher::ForceScan(int reportAll, int priority)
{
	return _call(API_WATCHER_FORCESCAN, (int)0, reportAll, priority);
}
inline void api_watcher::StopScan(void)
{
	_voidcall(API_WATCHER_STOPSCAN);
}
	
	
inline const wchar_t* api_watcher::GetID(void)
{
	return _call(API_WATCHER_GET_ID, (const wchar_t*)NULL);
}
inline const wchar_t* api_watcher::GetPath(void)
{
	return _call(API_WATCHER_GET_PATH, (const wchar_t*)NULL);
}
inline int api_watcher::GetRecurse(void)
{
	return _call(API_WATCHER_GET_RECURSE, (int)0);
}
inline int api_watcher::GetTrackingMode(void)
{
	return _call(API_WATCHER_GET_TRACKMODE, (int)0);
}
inline const wchar_t* api_watcher::GetTrackingPath(void)
{
	return _call(API_WATCHER_GET_TRACKPATH, (const wchar_t*)NULL);
}
inline api_watchfilter* api_watcher::GetExtensionFilter(void)
{
	return _call(API_WATCHER_GET_EXTFILTER, (api_watchfilter*)NULL);
}
inline int api_watcher::GetExtensionFilterType(void)
{
	return _call(API_WATCHER_GET_EXTFILTERTYPE, (int)0);
}
inline size_t api_watcher::GetMinSizeFilter(void)
{
	return _call(API_WATCHER_GET_SIZEFILTER, (size_t)0);
}
inline int api_watcher::GetStatus(void)
{
	return _call(API_WATCHER_GET_STATUS, (int)0);
}

#endif // __WASABI_API_WATCHER_H