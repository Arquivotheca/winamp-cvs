#pragma once
#include "syscb/ifc_syscallback.h"
#include "foundation/mkncc.h"
#include "foundation/error.h"

namespace Gracenote 
{
	// {490C9B51-9CA7-408A-B0CF-E46287E98334}
static const GUID event_type = 
{ 0x490c9b51, 0x9ca7, 0x408a, { 0xb0, 0xcf, 0xe4, 0x62, 0x87, 0xe9, 0x83, 0x34 } };
	
	static const int on_status  = 0;
	static const int on_error   = 1;
	class SystemCallback : public ifc_sysCallback
	{
	public:
			enum
	{
		STATUS_GRACENOTE_MANAGER_LOADING=0x0000,
		STATUS_GRACENOTE_MANAGER_LOADED=0x000F,
		STATUS_GRACENOTE_USER_LOADING=0x0010,
		STATUS_GRACENOTE_USER_REGISTRATION=0x0011,
		STATUS_GRACENOTE_USER_LOADED=0x001F,
		STATUS_GRACENOTE_LOCALE_LOADING=0x020,
		STATUS_GRACENOTE_LOCALE_DOWNLOADING=0x0021,
		STATUS_GRACENOTE_LOCALE_LOADED=0x002F,
		STATUS_GRACENOTE_DSP_LOADING = 0x0030,
		STATUS_GRACENOTE_DSP_LOADED = 0x003F,
		STATUS_GRACENOTE_LINK_LOADING = 0x0040,
		STATUS_GRACENOTE_LINK_LOADED = 0x004F,
		STATUS_GRACENOTE_MUSICID_FILE_LOADING = 0x0050,
		STATUS_GRACENOTE_MUSICID_FILE_LOADED = 0x005F,
		STATUS_GRACENOTE_LOADED=0x0FFF,


	};
	protected:
		SystemCallback() {}
		~SystemCallback() {}
		GUID WASABICALL SysCallback_GetEventType() { return event_type; }
		int WASABICALL SysCallback_Notify(int msg, intptr_t param1, intptr_t param2)
		{
			int status_message;
			ns_error_t error_code;
			switch(msg)
			{
			case on_status:
				status_message = (int)param1;
				return GracenoteSystemCallback_OnStatus(status_message);
			case on_error:
				status_message = (int)param1;
				error_code = (ns_error_t)param2;
				return GracenoteSystemCallback_OnError(status_message, error_code);
			default:
				return NErr_Success;
			}
		}
		virtual int WASABICALL GracenoteSystemCallback_OnStatus(int status_message) { return NErr_Success; }
		/* this will get called with the phase that the error happened on */
		virtual int WASABICALL GracenoteSystemCallback_OnError(int status_message, ns_error_t error_code) { return NErr_Success; }
	};

}