#ifndef NULLSOFT_PATHTRACKING_INI_HEADER
#define NULLSOFT_PATHTRACKING_INI_HEADER

#include <windows.h>
#include ".\ini.h"

#include ".\watchScanner.h"

class PathTrackINI : protected BaseINI
{

public:
	PathTrackINI(void);
	virtual ~PathTrackINI(void);

public:
	HRESULT SetTracker(const wchar_t *file, const wchar_t *watcherID);
	HRESULT WritePathInfo(PATHINFO *pathInfo);
	HRESULT GetPathInfo(PATHINFO *pathInfo);
	unsigned __int32* GetPathHashList(unsigned int *size);
	HRESULT DeletePathRec(unsigned __int32 pathHash);
	void FreeHashList(void* data);

protected:
	MLString record;
};

#endif // NULLSOFT_PATHTRACKING_INI_HEADER