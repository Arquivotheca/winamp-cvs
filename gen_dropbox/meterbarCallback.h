#ifndef NULLOSFT_DROPBOX_PLUGIN_METERBARCALLBACK_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_METERBARCALLBACK_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

class Meterbar;
class Document;
class DropboxView;

class __declspec(novtable) MeterbarCallback
{

protected:
	MeterbarCallback(){}
	virtual ~MeterbarCallback(){}

public:
	virtual void OnDestroy(Meterbar *instance) = 0;
	virtual void Invalidate(const RECT *prcInvalid) = 0;
	virtual void ShowTip(LPCTSTR pszText, const RECT *prcBounds) = 0;
	virtual Document *GetDocument() = 0;
	virtual DropboxView *GetView() = 0;
	virtual void MetricsInvalid() = 0; // you need to call UpdateMetrics() better do it on timer
};


#endif //NULLOSFT_DROPBOX_PLUGIN_METERBARCALLBACK_HEADER