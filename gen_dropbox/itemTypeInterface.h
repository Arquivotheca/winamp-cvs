#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEMTYPE_INTERFACE_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEMTYPE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IFileInfo;

EXTERN_C const IID IID_IItemType;


#define NO_ICON		((UINT)-1)

MIDL_INTERFACE("9A3C8B93-AA84-464b-BFF6-236A6CD45D74")
IItemType : public IUnknown
{

public:

	typedef enum 
	{
		itemTypeMissingFile = 0,
		itemTypeUnknown = 1,
		itemTypeAudioFile = 2,
		itemTypeVideoFile = 3,
		itemTypePlaylistFile = 4,
		itemTypeFolder = 5,
		itemTypeLinkFile = 6,
		itemTypeHttpStream = 7,
		itemTypeAudioCdTrack = 8,
	} KnownItemTypes;

	typedef enum
	{
		typeCapPlay		= 0x0001,
		typeCapCopy		= 0x0002,
		typeCapEnumerate	= 0x0004,
	} TypeCapabilities;

	virtual BYTE STDMETHODCALLTYPE GetId(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetName(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDisplayName(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetDescription(LPTSTR pszBuffer, INT cchBufferMax) = 0;
	virtual UINT STDMETHODCALLTYPE GetCapabilities(void) = 0;
	virtual UINT STDMETHODCALLTYPE GetIconId(void) = 0;
	virtual HRESULT STDMETHODCALLTYPE CreateItem(LPCTSTR pszFilePath, WIN32_FILE_ATTRIBUTE_DATA *pAttributeData, IFileInfo **ppItem) = 0;
};

#endif //NULLSOFT_DROPBOX_PLUGIN_ITEMTYPE_INTERFACE_HEADER

