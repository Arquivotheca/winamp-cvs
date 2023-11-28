#include "api.h"
#include "config.h"
#include <api/syscb/callbacks/consolecb.h>
#include <shlwapi.h>
#include "../nu/AutoChar.h"
#include "../nu/AutoLock.h"
#include <strsafe.h>

FILE *flog = 0;
Nullsoft::Utility::LockGuard logguard;

void Log(const wchar_t *format, ...)
{
	Nullsoft::Utility::AutoLock loglock(logguard);
	wchar_t buffer[2048]; // hope it's big enough :)
	va_list args;
	va_start (args, format);
	StringCbVPrintf(buffer, sizeof(buffer), format, args);
	WASABI_API_SYSCB->syscb_issueCallback(SysCallback::CONSOLE, ConsoleCallback::DEBUGMESSAGE, 0, reinterpret_cast<intptr_t>(buffer));
	if (config_log)
	{
		if (!flog)
		{
			wchar_t log_filename[MAX_PATH];
			const wchar_t *user_path = WASABI_API_APP->path_getUserSettingsPath();
			PathCombineW(log_filename, user_path, L"orgler.log");

			flog = _wfopen(log_filename, L"a+");
			if (ftell(flog) == 0)
			{
				uint8_t BOM[3] = {0xEF, 0xBB, 0xBF};
				fwrite(BOM, 3, 1, flog);
			}
		}
		fprintf(flog, "%s\n", AutoChar(buffer, CP_UTF8), flog);
		fflush(flog);
	}
	va_end(args);
}

void CloseLog()
{
	if (flog)
		fclose(flog);
	flog=0;
}

const wchar_t *MakeDateString(__time64_t convertTime)
{
	static wchar_t dest[2048];

	SYSTEMTIME sysTime;
	tm *newtime = _localtime64(&convertTime);
	if (newtime)
	{
		sysTime.wYear	= (WORD)(newtime->tm_year + 1900);
		sysTime.wMonth	= (WORD)(newtime->tm_mon + 1);
		sysTime.wDayOfWeek = (WORD)newtime->tm_wday;
		sysTime.wDay		= (WORD)newtime->tm_mday;
		sysTime.wHour	= (WORD)newtime->tm_hour;
		sysTime.wMinute	= (WORD)newtime->tm_min;
		sysTime.wSecond	= (WORD)newtime->tm_sec;
		sysTime.wMilliseconds = 0;

		GetTimeFormatW(LOCALE_USER_DEFAULT, 0, &sysTime, NULL, dest, 2048);

		//wcsftime(expire_time, 63, L"%b %d, %Y", _localtime64(&convertTime));
	}
	else
		dest[0] = 0;

	return dest;
}

