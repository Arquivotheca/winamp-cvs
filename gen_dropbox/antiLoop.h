#ifndef NULLSOFT_DROPBOX_PLUGIN_ANTILOOP_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ANTILOOP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../nu/ptrList.h"

interface  IFileInfo;

class AntiLoop
{
public:
	AntiLoop();
	~AntiLoop();
public:
	BOOL Check(IFileInfo *pInfo);
	BOOL Check(LPCTSTR pszPath);
protected:
	typedef nu::PtrList<TCHAR> PATHLIST;
	PATHLIST sorted;
	PATHLIST accumulator;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_ANTILOOP_HEADER