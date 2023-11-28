#ifndef NULLSOFT_DROPBOX_PLUGIN_CLIPBOARDFORMAT_PROCESSOR_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_CLIPBOARDFORMAT_PROCESSOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IFileEnumerator;

EXTERN_C const IID IID_IClipboardFormatProcessor;

MIDL_INTERFACE("5ED72580-DC74-4e40-9541-C3CB725364E0")
IClipboardFormatProcessor : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Process(INT iInsert) = 0;
};

typedef enum
{
	DATAOBJECT_HDROP = 0x0001,
	DATAOBJECT_VIEWITEMS = 0x0002,
} DATAOBJECTFORMATS;

HRESULT CreateDataObectProcessor(IDataObject *pDataObject, HWND hwndDropBox, IClipboardFormatProcessor **ppProcessor, UINT useFormats);

#endif //NULLSOFT_DROPBOX_PLUGIN_CLIPBOARDFORMAT_PROCESSOR_INTERFACE_HEADER