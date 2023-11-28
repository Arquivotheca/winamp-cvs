#pragma once

#include <bfc/dispatch.h>

class api_logger :
    public Dispatchable
{
public:
    
    void Lock();
    void Unlock();
    void LogMsg(            // Log a full logger Message
        wchar_t *plId,
        wchar_t severity,
        int code,
        wchar_t *text,
        wchar_t *filename,
        wchar_t *group
    );                      
    void *LogEnum(void *context, void **retVal);


	DISPATCH_CODES
	{
      API_LOGGER_LOCK = 10,
      API_LOGGER_UNLOCK = 20,
      API_LOGGER_LOGMSG = 30,
      API_LOGGER_LOGENUM = 40
	};
};

inline void api_logger::LogMsg(
        wchar_t *plId,
        wchar_t severity,
        int code,
        wchar_t *text,
        wchar_t *filename,
        wchar_t *group
    ) 
{
    _voidcall(API_LOGGER_LOGMSG,
        plId,
        severity,
        code,
        text,
        filename,
        group
        );
}

inline void *api_logger::LogEnum(void *context, void **retVal)
{
    return _call(API_LOGGER_LOGENUM, (void *)0, context, retVal);
}

inline void api_logger::Lock()
{
	_voidcall(API_LOGGER_LOCK);
}
inline void api_logger::Unlock()
{
	_voidcall(API_LOGGER_UNLOCK);
}

