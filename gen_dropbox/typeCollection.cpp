#include "main.h"
#include "./typeCollection.h"
#include "./typeObject.h"
#include "./supportedExtensions.h"
#include "./resource.h"


#include "./fiUnknown.h"
#include "./fiFolder.h"
#include "./fiPlaylist.h"
#include "./fiAudio.h"
#include "./fiVideo.h"
#include "./fiMissing.h"
#include "./fiStream.h"
#include "./fiLink.h"
#include "./fiAudioCdTrack.h"

#include <shobjidl.h>
#include <shlwapi.h>


typedef struct __KNOWNTYPEDATA
{
	BYTE id;
	LPCSTR name;
	UINT iconId;
	INT displayResourceId;
	INT descResourceId;
	UINT capabilities;
	TypeObject::CreatorProc createProc;
} KNOWNTYPEDATA;


static const KNOWNTYPEDATA knownTypes[] = 
{
	{ IItemType::itemTypeAudioFile, "audioFile", IItemType::itemTypeAudioFile,
		IDS_FILETYPE_AUDIO, IDS_FILETYPE_AUDIO, 
		IItemType::typeCapCopy | IItemType::typeCapPlay,
		AudioFileInfo::CreateInstance },
	
	{ IItemType::itemTypeVideoFile, "videoFile", IItemType::itemTypeVideoFile,
		IDS_FILETYPE_VIDEO, IDS_FILETYPE_VIDEO, 
		IItemType::typeCapCopy | IItemType::typeCapPlay,
		VideoFileInfo::CreateInstance },
	
	{ IItemType::itemTypeMissingFile, "missingFile", IItemType::itemTypeMissingFile,
		IDS_FILETYPE_MISSING, IDS_FILETYPE_MISSING, 
		0,
		MissingFileInfo::CreateInstance},
	
	{ IItemType::itemTypeUnknown, "unknownItem", IItemType::itemTypeUnknown,
		IDS_FILETYPE_UNKNOWN, IDS_FILETYPE_UNKNOWN, 
		IItemType::typeCapCopy,
		UnknownFileInfo::CreateInstance },
	
	{ IItemType::itemTypePlaylistFile, "playlistFile", IItemType::itemTypePlaylistFile,
		IDS_FILETYPE_PLAYLIST, IDS_FILETYPE_PLAYLIST, 
		IItemType::typeCapCopy | IItemType::typeCapEnumerate,
		PlaylistFileInfo::CreateInstance},
	
	{ IItemType::itemTypeFolder, "folder", IItemType::itemTypeFolder,
		IDS_FILETYPE_FOLDER, IDS_FILETYPE_FOLDER, 
		IItemType::typeCapCopy | IItemType::typeCapEnumerate,
		FolderFileInfo::CreateInstance },
	
	{ IItemType::itemTypeLinkFile, "linkFile", IItemType::itemTypeLinkFile,
		IDS_FILETYPE_LINK, IDS_FILETYPE_LINK, 
		IItemType::typeCapCopy | IItemType::typeCapEnumerate,
		LinkFileInfo::CreateInstance },
	
	{ IItemType::itemTypeHttpStream, "httpStream", IItemType::itemTypeHttpStream,
		IDS_FILETYPE_STREAM, IDS_FILETYPE_STREAM, 
		IItemType::typeCapPlay,
		StreamFileInfo::CreateInstance},

	{ IItemType::itemTypeAudioCdTrack, "audioCdTrack", IItemType::itemTypeAudioCdTrack,
		IDS_FILETYPE_AUDIOCDTRACK, IDS_FILETYPE_AUDIOCDTRACK, 
		IItemType::typeCapCopy | IItemType::typeCapPlay,
		AudioCdTrackInfo::CreateInstance },
};

TypeCollection::TypeCollection()
{
}

TypeCollection::~TypeCollection()
{
	size_t index = typeList.size();
	while(index--)
		typeList[index]->Release();
}

void TypeCollection::RegisterKnownTypes()
{
	IItemType *instance;
	const KNOWNTYPEDATA *data;

	for (INT i = 0; i < ARRAYSIZE(knownTypes); i++)
	{
		data = &knownTypes[i];
		instance = new TypeObject(data->id, data->name, data->iconId,
						MAKEINTRESOURCE(data->displayResourceId), MAKEINTRESOURCE(data->descResourceId),
						data->capabilities, data->createProc);

		if (NULL != instance) typeList.push_back(instance);	
	}
}


HRESULT TypeCollection::CreateItem(LPCTSTR filePath, WIN32_FILE_ATTRIBUTE_DATA *fileAttributes, IFileInfo **ppItem)
{
	HRESULT hr;
	WIN32_FILE_ATTRIBUTE_DATA attributes;

	if (NULL == filePath || NULL == ppItem)
		return E_POINTER;
	
	if (PathIsURL(filePath))
	{
		TCHAR szCdaMarker[] = TEXT("cda://");
		INT cchLen = lstrlen(filePath);
		if (cchLen > ARRAYSIZE(szCdaMarker) && CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, 
					szCdaMarker, ARRAYSIZE(szCdaMarker) -1, filePath, ARRAYSIZE(szCdaMarker) - 1))
		{
			return AudioCdTrackInfo::CreateInstance(filePath, NULL, ppItem);
		}
		return StreamFileInfo::CreateInstance(filePath, NULL, ppItem);
	}

	LPCTSTR p, c;
	static LPCTSTR invalidChars = TEXT("*?\"<>|");

	for (p = filePath; TEXT('\0') != *p; p++)
	{		
		for (c = invalidChars; TEXT('\0') != *c && *c != *p; c++);
		if (TEXT('\0') != *c) return E_INVALIDARG;
	}
	
	for (p = filePath; TEXT('.') == *p && TEXT('\0') != *p; p++);
	if (TEXT('\0') == *p) 
		return E_INVALIDARG;

	if (NULL != fileAttributes)
	{
		CopyMemory(&attributes, fileAttributes, sizeof(WIN32_FILE_ATTRIBUTE_DATA));
	}
	else
	{
		hr = UnknownFileInfo::ReadFileAttributes(filePath, &attributes);
		if (FAILED(hr)) 
		{
			// AudioCd intercept
			LPCTSTR pszExtension = PathFindExtension(filePath);
			if (NULL != pszExtension && pszExtension != filePath && TEXT('.') == *pszExtension)
			{
				pszExtension++;
				if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, TEXT("cda"), -1, pszExtension, -1))
				{
					return AudioCdTrackInfo::CreateInstance(filePath, NULL, ppItem);
				}
			}
			return MissingFileInfo::CreateInstance(filePath, NULL, ppItem);
		}
	}

	if (0 != (FILE_ATTRIBUTE_DIRECTORY & attributes.dwFileAttributes)) 
	{
		return FolderFileInfo::CreateInstance(filePath, &attributes, ppItem);
	}
		
	HANDLE hSupportedExt = GetDefaultSupportedExtensionsHandle();
	
	if (NULL != hSupportedExt)
	{
		LPCTSTR pszExtension = PathFindExtension(filePath);
		if (NULL != pszExtension && pszExtension != filePath && TEXT('.') == *pszExtension)
		{
			pszExtension++;
		
			IItemType *type = GetTypeByExtension(hSupportedExt, pszExtension, NULL);
			if (NULL != type)
				return type->CreateItem(filePath, &attributes, ppItem);

		}
	}
	return UnknownFileInfo::CreateInstance(filePath, &attributes, ppItem);
}

IItemType *TypeCollection::FindById(BYTE typeId)
{
	size_t index = typeList.size();
	while(index--)
	{
		if (typeList[index]->GetId() == typeId)
			return typeList[index];
	}
	return NULL;
}

INT TypeCollection::Count()
{
	return (INT)typeList.size();
}

BOOL TypeCollection::Enumerate(TypeEnumProc proc, ULONG_PTR param)
{
	if (NULL == proc)
		return FALSE;

	size_t count = typeList.size();
	for(size_t i = 0; i < count; i++)
	{
		if (!proc(typeList[i], param)) 
			return FALSE;
	}
	return TRUE;
}

IItemType *TypeCollection::FindByName(LPCTSTR pszName, INT cchName)
{
	if (NULL == pszName || 0 == typeList.size()) return NULL;
	if (cchName < 0) cchName = lstrlen(pszName);
	if (0 == cchName) return NULL;

	TCHAR szName[128];
	size_t count = typeList.size();
	for(size_t i = 0; i < count; i++)
	{
		if (SUCCEEDED(typeList[i]->GetName(szName, ARRAYSIZE(szName))) &&
			CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szName, -1, pszName, cchName)) 
		{
			return typeList[i];
		}
	}

	return NULL;
}