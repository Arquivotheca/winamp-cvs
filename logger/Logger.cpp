#include "Logger.h"
#include "api.h"
#include <time.h>
#include "../nu/AutoLock.h"
#include "..\Winamp\wa_ipc.h"
#include <api/service/waservicefactory.h>
#include <strsafe.h>

using namespace Nullsoft::Utility;

const wchar_t * LOG_FILENAME = L"Winamp.log";

Logger::Logger() : toFile(false), logFile(0) {};
Logger::~Logger()
{
    CloseLogFile();
}

void Logger::LogToFile(boolean toFile)
{
    this->toFile = toFile;
}

void Logger::Log(wchar_t *msg)
{
    MessageBox(NULL,msg,L"Info2",MB_OK);
};

void Logger::LogMsg(
   wchar_t *plId,
   wchar_t severity,
   int code,
   wchar_t *text,
   wchar_t *filename,
   wchar_t *group
   )
{
   	AutoLockT<Logger> lock (this);

    HRESULT hr;
    size_t plId_size = 0;
    LogItem *logEntry = new LogItem;
    if (!logEntry) return;

    logEntry->itemTime = time(NULL);

    // load pluginId
    hr = StringCchLength(plId, 21, &plId_size);
    if ( SUCCEEDED(hr) && plId_size > 0)
    {
        logEntry->pluginId = _wcsdup(plId);
    }
    // load severity
    logEntry->severity = severity;

    // load code
    logEntry->code = code;

    // load text
    size_t text_size;
    hr = StringCchLength(text, 100, &text_size);
    if ( SUCCEEDED(hr) && text_size > 0)
    {
        logEntry->text = _wcsdup(text);
    }

    // load filename
    size_t filename_size;
    hr = StringCchLength(filename, MAX_PATH, &filename_size);
    if ( SUCCEEDED(hr) && filename_size > 0)
    {
        logEntry->filename = _wcsdup(filename);
    }

    // load group
    size_t group_size;
    hr = StringCchLength(group, 10, &group_size);
    if ( SUCCEEDED(hr) && group_size > 0)
    {
        logEntry->group = _wcsdup(group);
    }

    logDequeue.push_front(logEntry);

    if (toFile)
    {
        if (OpenLogFile())
        {
            tm *ptm = new tm();
            gmtime_s(ptm, &(logEntry->itemTime));
            wchar_t ltime[21];
            memset(ltime,0,sizeof(ltime));
            StringCchPrintf(ltime,sizeof(ltime),L"%d/%d/%d %d:%d:%d",
                ptm->tm_mon + 1,ptm->tm_mday,ptm->tm_year + 1900,ptm->tm_hour,ptm->tm_min,ptm->tm_sec);

            fwprintf(logFile,L"%s %s %s %d %c %s %s",
                ltime,
                logEntry->group,
                logEntry->filename,
                logEntry->code,
                logEntry->severity,
                logEntry->pluginId,
                logEntry->text
                );

            fflush(logFile);
        }
    }
}

bool Logger::OpenLogFile()
{
    if (!logFile)
    {
	    waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	    if (sf)
		    WASABI_API_APP = (api_application *)sf->getInterface();

        const wchar_t *logPath = WASABI_API_APP->path_getUserSettingsPath();

        int logPathSize = lstrlen(logPath) + lstrlen(LOG_FILENAME) + 2;
        wchar_t *fullLogPath = new wchar_t[logPathSize];
        StringCchCopy(fullLogPath,logPathSize,logPath);
        StringCchCat(fullLogPath,logPathSize,L"\\");
        StringCchCat(fullLogPath,logPathSize,LOG_FILENAME);
        
        logFile = _wfopen(fullLogPath,L"wt");
    }

    if (!logFile)
        return false;
    else
        return true;
}

void Logger::CloseLogFile()
{
    if (logFile)
        fclose(logFile);
}

void * Logger::LogEnum(void *ctx, void **retVal)
{
   	AutoLockT<Logger> lock (this);

    LogItem *retLogItem = NULL;
    if (!ctx)
    {
        retLogItem = logDequeue.front();
    }
    else
    {
        if ( ((LogItem *)ctx)->next != NULL )
            retLogItem = (LogItem *)((LogItem *)ctx)->next;
    }

    *retVal = (void *)retLogItem;

    return (void *)retLogItem;
}

void Logger::Lock()
{
    loggerGuard.Lock();
}

void Logger::Unlock()
{
    loggerGuard.Unlock();
}


#define CBCLASS Logger
START_DISPATCH;
VCB(API_LOGGER_LOGMSG, LogMsg);
CB(API_LOGGER_LOGENUM, LogEnum);
VCB(API_LOGGER_LOCK, Lock);
VCB(API_LOGGER_UNLOCK, Unlock);

END_DISPATCH;
#undef CBCLASS

