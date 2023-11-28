#ifndef NULLSOFT_ML_PLAYLIST_AMN
#define NULLSOFT_ML_PLAYLIST_AMN

#include "windows.h"
#include "./api.h"
#include <api/service/waServiceFactory.h>
#include "../watcher/api_watcher.h"

class AMNWatcher
{
public:
	AMNWatcher(void);
	~AMNWatcher(void);

public:
	int Init(api_service *service, const wchar_t *trackPath);
	void Destroy(void);
protected:
	static int OnWatcherNotify(api_watcher *sender, UINT message, LONG_PTR param, void* userdata);

protected:
	static waServiceFactory *watcherFactory;
	api_watcher *watcherOld;
	api_watcher *watcherNew;
	BOOL dirty;
};

#endif // NULLSOFT_ML_PLAYLIST_AMN
