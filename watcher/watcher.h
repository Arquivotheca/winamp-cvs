#ifndef NULLSOFT_ML_WATCHER_HEADER
#define NULLSOFT_ML_WATCHER_HEADER

#include ".\api_watcher.h"
#include ".\watchFilter.h"
#include ".\watchScanner.h"
#include ".\MLString.h"
#include ".\FileTrackINI.h"
#include ".\PathTrackINI.h"


#define TRACK_DIR					L"tracking"
#define TRACK_INI_NAME				L"winamp.trk"
#define TRACK_PATH_INI_NAME			L"path.trk"
#define TRACK_INI_NAME_LEN			12
#define TRACK_INI_RECORD_LEN		33
#define TRACK_INI_ALLOC_STEP		256



// Class MLWatcher
class MLWatcher : public api_watcher
{
	friend class MLWatchScanner;
	friend class MLWatchXML;
	public:
		MLWatcher(void);
		virtual ~MLWatcher(void);

	public: 
		int CreateWatch(const wchar_t *id, const wchar_t *path, int recurse, int trackMode, WATCHERNOTIFY cbNotify);
		void DestroyWatch(void);
		int Parse(const void* xml, unsigned int length, const wchar_t* encoding);
		MLWatcher* CopyTo(MLWatcher *destination);
		wchar_t *GetString(wchar_t *buffer, unsigned int bufferLen);
		unsigned int GetStringLength(void);

		int SetExtensionFilter(const wchar_t *string, int filterType);
		void SetExtensionFilterType(int type);
		void SetMinSizeFilter(size_t size);
		void SetTrackingPath(const wchar_t* path);
		WATCHERNOTIFY SetCallBack(WATCHERNOTIFY cbNotify); 
		void* SetUserData(void* data);

		int Start(void);
		void Stop(void);

		int ForceScan(int reportAll, int priority);
		void StopScan(void);

	public: 
		int GetStatus(void) {return status; }
		const wchar_t* GetPath(void) { return path; }
		int GetRecurse(void) { return recurse; }
		int GetTrackingMode(void) { return trackMode; }
		int GetExtensionFilterType(void) { return fltExtType; }
		api_watchfilter* GetExtensionFilter(void) { return &fltExt; }
		size_t GetMinSizeFilter(void) { return fltSize; }
		const wchar_t* GetID(void) { return id; }
		const wchar_t* GetTrackingPath(void) { return ctPath; } // return path to the central tracking

	protected:
		void ProcessFolder(PATHINFO *info, BOOL reportNoChanged);
		void ProcessFolderList(Vector<unsigned __int32> *folderList);
		int Notify(int type, LONG_PTR param);
		void CalculateTrackFileName(MLString *iniFile, unsigned __int32 pathHash, MLString *path);
		void SetWatcherChangeInfo(WATCHERCHANGEINFO *wcInfo, PATHINFO *pathInfo, FILEINFO *fileInfo, int state);
		unsigned int GetHash(const wchar_t *str, size_t len);
		static unsigned long _stdcall WatchThread(void* param);

				
	private:
		void			*heap;
		void			*thread;
		void			*evntStopWatch;
		
		wchar_t			*id;
		int				status;
		
		wchar_t			*path;			// path to watch

		MLWatchFilter	fltExt;		// extension filter to use when looking for files
		int				fltExtType;		// tpye of the extension filter 
		size_t			fltSize;		// minimal size filter to use when looking for files

		int				recurse;		// allow search subfolders
		int				trackMode;		// specify tracking state changes mode (can be one of the WATCHER_TRACKMODE_*).

		MLString		ctPath;			// Path to the central tracking folder

		void			*userData;		// any user data pointer;		
		WATCHERNOTIFY	notify;			// callback to notify
		
		MLWatchScanner	scanner;
		MLString		iniName;
		FileTrackINI	fileTrack;
		PathTrackINI	pathTrack;

		

	protected:
		RECVS_DISPATCH;

};

#endif //NULLSOFT_ML_WATCHER_HEADER