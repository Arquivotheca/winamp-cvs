#include "api_logger.h"
#include "../nu/PtrDeque2.h"
#include "../nu/AutoLock.h"

struct LogItem : public nu::PtrDequeNode
{
    time_t itemTime;
    wchar_t *pluginId;
    wchar_t severity;
    int code;
    wchar_t *text;
    wchar_t *filename;
    wchar_t *group;
//  wchar_t persistant;
}; 

static nu::PtrDeque2<LogItem> logDequeue;
static Nullsoft::Utility::LockGuard loggerGuard;

class Logger : api_logger
{
public:
    Logger();
    ~Logger();

   	void Lock();
	void Unlock();

    void LogToFile(boolean toFile);
    void Log(wchar_t *msg);
    void LogMsg(
        wchar_t *plId,
        wchar_t severity,
        int code,
        wchar_t *text,
        wchar_t *filename,
        wchar_t *group
        );
    void * LogEnum(void *ctx, void **retVal);

private:
    boolean toFile;
    FILE *logFile;


    bool OpenLogFile();
    void CloseLogFile();

    RECVS_DISPATCH;
};

