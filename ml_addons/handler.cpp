#include "service.h"
#include "navigation.h"
#include "handler.h"
#include "../Agave/URIHandler/svc_urihandler.h"
#include <api/service/waservicefactory.h>
#include "api.h"

int AddonsURIHandler::ProcessFilename(const wchar_t *filename)
{
	if (!_wcsnicmp(filename, L"winamp://Winamp Add-ons", 23) || !_wcsnicmp(filename, L"winamp://Winamp%20Add-ons", 25))
	{
		size_t index = 0;
		if (filename[15] == L' ')
			index = 23;
		else
			index = 25;
		wchar_t fullUrl[1024] = L"http://client.winamp.com/addons";
		if (filename[index] != 0)
		{
			StringCchCatW(fullUrl, 1024, filename + index);
		}
		Navigation_ShowService(SERVICE_ID, fullUrl, 
						NAVFLAG_FORCEACTIVE | NAVFLAG_ENSUREMLVISIBLE | NAVFLAG_ENSUREITEMVISIBLE);
		return HANDLED_EXCLUSIVE;
	}
	return NOT_HANDLED;
}

int AddonsURIHandler::IsMine(const wchar_t *filename)
{
	if (!_wcsnicmp(filename, L"winamp://Winamp Add-ons", 23 )  || !_wcsnicmp(filename, L"winamp://Winamp%20Add-ons", 25))
		return HANDLED;
	else
		return NOT_HANDLED;
}

#define CBCLASS AddonsURIHandler
START_DISPATCH;
CB(PROCESSFILENAME, ProcessFilename);
CB(ISMINE, IsMine);
END_DISPATCH;
#undef CBCLASS