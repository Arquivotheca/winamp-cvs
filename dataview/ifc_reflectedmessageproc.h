#ifndef _NULLSOFT_WINAMP_DATAVIEW_REFLECTED_MESSAGE_PROC_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_REFLECTED_MESSAGE_PROC_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {654F3592-C6D0-45fd-B177-926C85972CBD}
static const GUID IFC_ReflectedMessageProc = 
{ 0x654f3592, 0xc6d0, 0x45fd, { 0xb1, 0x77, 0x92, 0x6c, 0x85, 0x97, 0x2c, 0xbd } };


#include <bfc/dispatch.h>


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_reflectedmessageproc : public Dispatchable
{

protected:
	ifc_reflectedmessageproc() {}
	~ifc_reflectedmessageproc() {}

public:
	// ReflectedMessage:
	// return S_OK if you processed message and want to return result;
	HRESULT ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result);
	
	
public:
	DISPATCH_CODES
	{
		API_REFLECTEDMESSAGE = 10,
	};
};

inline HRESULT ifc_reflectedmessageproc::ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
	return _call(API_REFLECTEDMESSAGE, (HRESULT)E_NOTIMPL, message, wParam, lParam, result);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_REFLECTED_MESSAGE_PROC_INTERFACE_HEADER