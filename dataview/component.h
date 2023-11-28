#ifndef _NULLSOFT_WINAMP_DATAVIEW_COMPONENT_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COMPONENT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "../winamp/api_wa5component.h"

class api_dataview;

class Component : public ifc_wa5component
{
public:
	Component();
	~Component();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_wa5component */
	void RegisterServices(api_service *service);
	void DeregisterServices(api_service *service);

	HRESULT GetDataView(api_dataview **instance);

protected:
	void ReleaseServices(void);

	void Lock();
	void Unlock();

protected:
	RECVS_DISPATCH;

private:
	CRITICAL_SECTION lock;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COMPONENT_HEADER