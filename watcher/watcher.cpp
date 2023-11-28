#include <windows.h>
#include <strsafe.h>
#include ".\watcher.h"
#include ".\watchXML.h"

#define WATCHER_SCANDELAY		5000

#define FILTER_SEPARATOR		L';'	
#define FILTER_ENDCHAR			0x0000	

MLWatcher::MLWatcher(void) : path(NULL), fltSize(0), id(NULL), recurse(TRUE), trackMode(WATCHER_TRACKMODE_INPLACE),
							notify(NULL),fltExtType(WATCHER_FILTERTYPE_NONE), thread(NULL)
{
	heap = GetProcessHeap();
	evntStopWatch = CreateEvent(NULL, TRUE, FALSE, NULL);
}

MLWatcher::~MLWatcher(void)
{
	DestroyWatch();
}

int MLWatcher::CreateWatch(const wchar_t *id, const wchar_t *path, int recurse, int trackMode, WATCHERNOTIFY cbNotify)
{
	if (!heap && !id) return FALSE;
	if (this->path) DestroyWatch();
	
	size_t len;
	len = lstrlenW(id) + 1;
	this->id = (wchar_t*) HeapAlloc(heap, NULL, len * sizeof(wchar_t));
	if (!id) return FALSE;
	StringCchCopyW(this->id, len, id);

	len = lstrlenW(path) + 1;
	this->path = (wchar_t*) HeapAlloc(heap, NULL, len * sizeof(wchar_t));
	if (!path) return FALSE;
	StringCchCopyW(this->path, len, path);
		
	this->recurse = recurse;
	this->trackMode = trackMode;
	this->notify = cbNotify;
	return TRUE;
}

void MLWatcher::DestroyWatch(void)
{
	if (id) { HeapFree(heap, NULL, id); id = NULL; }
	if (path) { HeapFree(heap, NULL, path);  path = NULL; }
	notify = NULL;
	fltExt.Clear();
	fltExtType = WATCHER_FILTERTYPE_NONE;
	fltSize = 0; 
	scanner.Stop(TRUE);
}

MLWatcher* MLWatcher::CopyTo(MLWatcher *destination)
{
	if (!destination->CreateWatch(id, path, recurse, trackMode, notify)) return NULL;
	
	fltExt.CopyTo(&destination->fltExt);
	destination->fltExtType = fltExtType;
	destination->SetMinSizeFilter(fltSize);
	ctPath.CopyTo(&destination->ctPath);
	return destination;
}

int MLWatcher::Parse(const void* xml, unsigned int length, const wchar_t* encoding)
{
	MLWatchXML parser;
	return parser.Parse(this, xml, length, encoding);
}

unsigned int MLWatcher::GetStringLength(void)
{
	MLWatchXML xml;
	unsigned int len = xml.GetStringLength(this);
	return len;
}
wchar_t *MLWatcher::GetString(wchar_t *buffer, unsigned int bufferLen)
{
	MLWatchXML xml;
	int retCode = xml.GetString(this, buffer, bufferLen);
	return (retCode) ? buffer : NULL;
}

void MLWatcher::SetExtensionFilterType(int type)
{ 
	fltExtType = type;
}

int MLWatcher::SetExtensionFilter(const wchar_t *string, int filterType)
{
	if (!heap) return FALSE;
	fltExt.Clear();
	fltExtType = filterType;
	return (WATCHER_FILTERTYPE_NONE == fltExtType) ? 1 : fltExt.AddString(string, FILTER_SEPARATOR, FILTER_ENDCHAR);
}

void MLWatcher::SetMinSizeFilter(size_t size)
{
	this->fltSize = size;
}

void MLWatcher::SetTrackingPath(const wchar_t* path)
{
	if (!path) return;
	ctPath.Allocate(lstrlenW(path) + lstrlenW(TRACK_DIR) + 2);
	ctPath.Format(L"%s\\%s", path, TRACK_DIR);
	CreateDirectoryW(ctPath, NULL); 
	MLString iniPath = (ctPath.GetLength() + lstrlenW(TRACK_PATH_INI_NAME) + 3);
	iniPath.Format(L"%s\\%s", (const wchar_t*)ctPath, TRACK_PATH_INI_NAME);
	pathTrack.SetTracker(iniPath, id);
}

WATCHERNOTIFY MLWatcher::SetCallBack(WATCHERNOTIFY cbNotify)
{
	WATCHERNOTIFY tmp = notify;
	notify = cbNotify;
	return tmp;
}
void* MLWatcher::SetUserData(void* data)
{
	void* tmp = userData;
	userData = data;
	return tmp;
}


void MLWatcher::CalculateTrackFileName(MLString *iniFile, unsigned __int32 pathHash, MLString *path)
{
	switch(trackMode)
	{
		case WATCHER_TRACKMODE_INPLACE:
			iniFile->Allocate(path->GetLength() + TRACK_INI_NAME_LEN + 3);
			iniFile->Format(L"%s\\%s", (wchar_t*)path, TRACK_INI_NAME);
			break;
		case WATCHER_TRACKMODE_CENTRAL:
			iniFile->Allocate(ctPath.GetLength() + TRACK_INI_NAME_LEN + 3);
			iniFile->Format(L"%s\\%08I32X.trk", (wchar_t*)ctPath, pathHash);
			break;
	}
	fileTrack.SetTracker(iniFile->GetBuffer(),id);
}


void MLWatcher::ProcessFolder(PATHINFO *info, BOOL reportNoChanged)
{
	CalculateTrackFileName(&iniName, info->hPath, &info->path);

	unsigned __int32 *keys = NULL;
	
	unsigned int oldSize = fileTrack.GetCount();
	if (oldSize != MAXDWORD) 
	{
		keys = (unsigned __int32*) HeapAlloc(heap, NULL, oldSize*sizeof(unsigned __int32));
		fileTrack.GetFilesHashList(keys, oldSize);
	}

	unsigned int realCount = 0;
	unsigned int length = (unsigned int)info->fileInfo.size();

	for (unsigned int i = 0; i < length; i++)
	{
		FILEINFO *file = info->fileInfo.at(i);
		// filters
		if (fltSize != 0 && file->sizeLow < fltSize) continue;
		if (!fltExt.IsEmpty())
		{
			BOOL filtered = fltExt.Check(file->name, file->name.GetLength());
			if ((WATCHER_FILTERTYPE_INCLUDE == fltExtType && !filtered) || (WATCHER_FILTERTYPE_EXCLUDE == fltExtType && filtered)) continue;
		}
		if (0 == lstrcmpW(file->name, TRACK_INI_NAME)) continue;
		
		// tracking
		realCount++;
		int state = WATCHER_FILESTATE_ADDED;
		if (keys)
		{
			FILEINFO fi;
			for (unsigned int i = 0; i < oldSize; i++)
			{				
				if (keys[i] == file->hash)
				{		
					fi.hash = file->hash;
					if (S_OK == fileTrack.GetFullFileInfo(&fi))
					{
						state =  (fi.sizeHigh == file->sizeHigh && fi.sizeLow == file->sizeLow && 
								fi.wTime.dwHighDateTime == file->wTime.dwHighDateTime && fi.wTime.dwLowDateTime == file->wTime.dwLowDateTime) ?
									WATCHER_FILESTATE_NOCHANGES : WATCHER_FILESTATE_CHANGED;
					}
					else state = WATCHER_FILESTATE_CHANGED;
					keys[i] = 0x0000;
					break;
				}
			}
		} 
		int retNotify = 1;		
		if (notify && (reportNoChanged || state != WATCHER_FILESTATE_NOCHANGES))
		{
			WATCHERCHANGEINFO wci;
			SetWatcherChangeInfo(&wci, info, file, state); 
			retNotify = Notify(WATCHER_MSG_FILE, (LONG_PTR)&wci);
		}

		if (WATCHER_FILESTATE_NOCHANGES != state && retNotify)  fileTrack.WriteFileInfo(file);
		
	}
	if (realCount > 0 || oldSize != MAXDWORD)
	{
		fileTrack.SetCount(realCount);
		pathTrack.WritePathInfo(info);
		// removed files scan 
		if(keys)
		{
			FILEINFO fi;
			for (unsigned int i = 0; i < oldSize; i++)
			{
				if (keys[i] != 0x0000)
				{ 
					fi.hash = keys[i];
					if(S_OK != fileTrack.GetFullFileInfo(&fi)) continue;
					
					WATCHERCHANGEINFO wci;
					SetWatcherChangeInfo(&wci, info, &fi, WATCHER_FILESTATE_REMOVED); 
					if (Notify(WATCHER_MSG_FILE, (LONG_PTR)&wci)) fileTrack.DeleteFileRec(keys[i]);
				}
			}
		}
	}
	else fileTrack.DeleteEmptyFile();
	if (keys) HeapFree(heap, NULL, keys);
	keys = NULL;
	
}
void MLWatcher::SetWatcherChangeInfo(WATCHERCHANGEINFO *wcInfo, PATHINFO *pathInfo, FILEINFO *fileInfo, int state)
{
	wcInfo->cchPath = pathInfo->path.GetLength();
	wcInfo->path		= pathInfo->path;
	wcInfo->hPath	= pathInfo->hPath;
	wcInfo->cchFile = fileInfo->name.GetLength();
	wcInfo->file		= fileInfo->name;
	wcInfo->hFile	= fileInfo->hash;
	wcInfo->sizeLow = fileInfo->sizeLow;
	wcInfo->sizeHigh= fileInfo->sizeHigh;
	wcInfo->wTime	= fileInfo->wTime;
	wcInfo->state	= state;
}

int MLWatcher::Notify(int type, LONG_PTR param)
{
	return (notify) ? notify(this, type, param, userData) : 0;
}
void MLWatcher::ProcessFolderList(Vector<unsigned __int32> *folderList)
{
	unsigned int size = 0;
	unsigned __int32 *hashList = pathTrack.GetPathHashList(&size);
	
	for (unsigned int i = 0; i < size; i++)
	{
		size_t j;
		for (j = 0; j < folderList->size() && folderList->at(j) != hashList[i]; j++) {}
		if (j == folderList->size())
		{
			PATHINFO pi;
			pi.hPath = hashList[i];
			if(S_OK == pathTrack.GetPathInfo(&pi))
			{
				CalculateTrackFileName(&iniName, pi.hPath, &pi.path);
				unsigned int count = fileTrack.GetCount();
				if (count != MAXDWORD)
				{
					unsigned __int32 *keys = (unsigned __int32*) HeapAlloc(heap, NULL, count*sizeof(unsigned __int32));
					fileTrack.GetFilesHashList(keys, count);

					for (unsigned int k = 0; k < count; k++)
					{
						FILEINFO fi;
						fi.hash = keys[k];
						fileTrack.GetFullFileInfo(&fi);
						WATCHERCHANGEINFO wci;
						SetWatcherChangeInfo(&wci, &pi, &fi, WATCHER_FILESTATE_REMOVED);
						if(Notify(WATCHER_MSG_FILE, (LONG_PTR)&wci))
						{ 
							fileTrack.DeleteFileRec(keys[k]);
							count--;
						}
					}
				}
				if (count == 0) 
				{
					fileTrack.DeleteSection();
					fileTrack.DeleteEmptyFile();
					pathTrack.DeletePathRec(hashList[i]);
				}else fileTrack.SetCount(count);
				
			}
		}
	}
	
	pathTrack.FreeHashList(hashList);

}
unsigned int MLWatcher::GetHash(const wchar_t *str, size_t len)
{
   UINT hash = 0;
   char *chr = (char*)str;
   len *= 2;

   for(size_t i = 0; i < len; chr++, i++)
   {
      hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ (*chr) ^ (hash >> 3)) :
                               (~((hash << 11) ^ (*chr) ^ (hash >> 5)));
   }
   return (hash & 0x7FFFFFFF);
}


int MLWatcher::Start(void)
{
	if (thread) return FALSE; // another watch is going 
	unsigned long tid;
	ResetEvent(evntStopWatch);
	thread = CreateThread(NULL, 0, WatchThread, this, CREATE_SUSPENDED, &tid);
	if(thread)	ResumeThread(thread);
	return (NULL != thread);
}
void MLWatcher::Stop(void)
{
	if (!thread) return;
	SetEvent(evntStopWatch);
	StopScan();

}

int MLWatcher::ForceScan(int reportAll, int priority)
{
	if (!scanner.CreateScanner(this, reportAll))  return 0;
	scanner.SetPriority(priority);
	return scanner.Start();
}

void MLWatcher::StopScan(void)
{
	scanner.Stop(TRUE);
}
unsigned long _stdcall MLWatcher::WatchThread(void* param)
{
	MLWatcher *watcher = (MLWatcher*)param;
	HANDLE handles[2];
	handles[0] = watcher->evntStopWatch;
	handles[1] = FindFirstChangeNotification(watcher->path, watcher->recurse,
												FILE_NOTIFY_CHANGE_FILE_NAME |
												FILE_NOTIFY_CHANGE_DIR_NAME |
												FILE_NOTIFY_CHANGE_SIZE |
												FILE_NOTIFY_CHANGE_LAST_WRITE |
												FILE_NOTIFY_CHANGE_ATTRIBUTES);
	 
	if (INVALID_HANDLE_VALUE == handles[1]) return 0;
	OutputDebugStringW(L"Watcher activated (ID:"); OutputDebugStringW(watcher->id);	OutputDebugStringW(L")\n");

	DWORD ret;
	DWORD delay = INFINITE;
    while(WAIT_FAILED != (ret = WaitForMultipleObjects(2, handles, FALSE, delay)))
	{
		if (WAIT_OBJECT_0 == ret) break;
		if (WAIT_TIMEOUT == ret)
		{
			// start scanner;
			delay = INFINITE;
			if (watcher->ForceScan(FALSE, SCANPRIORITY_LOW))
			{
				OutputDebugStringW(L"Scanner activated (ID:"); OutputDebugStringW(watcher->id); 	OutputDebugStringW(L")\n");
			}
			continue;
		}
		if (!FindNextChangeNotification(handles[1])) { ret = WAIT_FAILED; break;}
		delay = WATCHER_SCANDELAY;
	}

	FindCloseChangeNotification(handles[1]);
	return (ret != WAIT_FAILED);
}


#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS MLWatcher
START_DISPATCH;
CB(API_WATCHER_CREATE, CreateWatch)
VCB(API_WATCHER_DESTROY, DestroyWatch)
CB(API_WATCHER_PARSE, Parse)
CB(API_WATCHER_GETSTRINGLENGTH, GetStringLength)
CB(API_WATCHER_GETSTRING, GetString)
CB(API_WATCHER_COPYTO, CopyTo)
CB(API_WATCHER_START, Start)
VCB(API_WATCHER_STOP, Stop)
VCB(API_WATCHER_SET_EXTFILTERTYPE, SetExtensionFilterType)
CB(API_WATCHER_SET_EXTFILTER, SetExtensionFilter)
VCB(API_WATCHER_SET_SIZEFILTER, SetMinSizeFilter)
VCB(API_WATCHER_SET_TRACKPATH, SetTrackingPath)
CB(API_WATCHER_SET_USERDATA, SetUserData)
CB(API_WATCHER_SET_CALLBACK, SetCallBack)
CB(API_WATCHER_FORCESCAN, ForceScan)
VCB(API_WATCHER_STOPSCAN, StopScan)
CB(API_WATCHER_GET_ID, GetID)
CB(API_WATCHER_GET_PATH, GetPath)
CB(API_WATCHER_GET_RECURSE, GetRecurse)
CB(API_WATCHER_GET_TRACKMODE, GetTrackingMode)
CB(API_WATCHER_GET_TRACKPATH, GetTrackingPath)
CB(API_WATCHER_GET_EXTFILTER, GetExtensionFilter)
CB(API_WATCHER_GET_EXTFILTERTYPE, GetExtensionFilterType)
CB(API_WATCHER_GET_SIZEFILTER, GetMinSizeFilter)
CB(API_WATCHER_GET_STATUS, GetStatus)
END_DISPATCH;
#undef CBCLASS