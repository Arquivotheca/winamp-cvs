#include "./main.h"
#include "./cfpInterface.h"
#include "./cfpViewItems.h"
#include "./cfpHDrop.h"

HRESULT CreateDataObectProcessor(IDataObject *pDataObject, HWND hwndDropBox, IClipboardFormatProcessor **ppProcessor, UINT useFormats)
{
	HRESULT hr = S_FALSE;
	*ppProcessor = NULL;

	if (0 != (DATAOBJECT_VIEWITEMS & useFormats) && S_OK != hr && SUCCEEDED(hr))
	{
		hr = CfViewItemsProcessor::CanProcess(pDataObject, hwndDropBox);
		if (S_OK == hr) *ppProcessor = new CfViewItemsProcessor(pDataObject, hwndDropBox);
	}
	
	if (0 != (DATAOBJECT_HDROP & useFormats) && S_OK != hr && SUCCEEDED(hr))
	{
		hr = CfHDropProcessor::CanProcess(pDataObject, hwndDropBox);
		if (S_OK == hr) *ppProcessor = new CfHDropProcessor(pDataObject, hwndDropBox);
	}
	
	if (S_FALSE == hr)
		hr = E_NOTIMPL;
	else if (S_OK == hr && NULL == *ppProcessor)
		hr = E_OUTOFMEMORY;

	return hr;
}