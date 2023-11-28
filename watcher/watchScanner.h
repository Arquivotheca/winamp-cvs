#ifndef NULLSOFT_ML_WATCHSCANNER_HEADER
#define NULLSOFT_ML_WATCHSCANNER_HEADER

#include ".\api_watcher.h"
#include "..\nu\Vector.h"
#include ".\MLString.h"

class MLWatcher;



typedef struct
{
	unsigned __int32		sizeLow;		// File size low
    unsigned __int32		sizeHigh;	// File size high
	FILETIME				wTime;		// File write access time.
	unsigned int		hash;		// hash code 
	MLString				name;		// file name
} FILEINFO;

typedef struct 
{
	unsigned int		hPath;		// path hash code (use it:)).
	MLString				path;		// Path to the file.
	Vector<FILEINFO*>	fileInfo;	// all files for current folder
} PATHINFO;

class MLWatchScanner
{
	public: 
		MLWatchScanner(void);
		virtual ~MLWatchScanner(void);

	public:
		int CreateScanner(MLWatcher *watch, int force);
		void DestroyScanner(void);

		int Start(void);
		void Stop(int block);  // If block is TRUE - will wait untill thread termination, else just tell thread to exit

	public:
		const MLWatcher* GetWatcher(void) { return watch; }
		int GetPriority(void) { return priority; }
		int GetForce(void)	{ return force; }
		size_t GetScannedFiles(void) { return counterFile; }
		size_t GetScannedFolders(void)  { return counterFolder; }
		unsigned __int32 GetScannerID(void) { return id; }
		int GetStatus(void) { return status; }
		void SetPriority(int priority);

	private:
		void Reset(void);
		int ScanFolder(PATHINFO *info, Vector<unsigned __int32> *folderList);
		void IncreaseLevel(void);
		int DecreaseLevel(void* search, int rCode);
		void FireStatusEvent(int newStatus);
		static unsigned long _stdcall ScanThread(void* param);

	protected:
		void				*heap;
		void				*thread;
		void				*evntStopScan;
		void				*evntThreadExit;

		unsigned int		id;				// automaticly generated scanner id (used to detect removed files)
		MLWatcher			*watch;
		int					priority;
		int					force;
		
		int					status;			// MLW_SCANSTATUS_*
	
		WIN32_FIND_DATAW		ffData;
		unsigned __int32		searchLevel;

		
		size_t	counterFile;
		size_t counterFolder;
		
};


#endif //NULLSOFT_ML_WATCHSCANNER_HEADER