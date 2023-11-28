#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_FACTORY_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_FACTORY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class DataView;

class DataViewFactory : public waServiceFactory
{
public:
	DataViewFactory();
	~DataViewFactory();

public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);

public:
	HRESULT Register(api_service *service);
	HRESULT Unregister(api_service *service);
	HRESULT GetDataView(api_dataview **instance);

protected:
	RECVS_DISPATCH;

protected:
	DataView *object;
};



#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_VIEW_FACTORY_HEADER
