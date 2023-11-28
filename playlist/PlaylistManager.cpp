#include "main.h"
#include "resource.h"
#include "PlaylistManager.h"
#include "ifc_playlistloader.h"
#include "M3ULoader.h"
#include "M3UWriter.h"
#include "PLSWriter.h"
#include "M3U8Writer.h"
#include "B4SWriter.h"
#include "../nu/AutoChar.h"
#include "Playlist.h"
#include "../playlist/svc_playlisthandler.h"
#include "../playlist/Handler.h"
#include "../nu/AutoWide.h"
#include <shlwapi.h>
#include <algorithm>
#include "Playlist.h"
#include <api/service/services.h>
#include "api.h"
#include <api/service/waservicefactory.h>
#include "PlaylistCounter.h"
#include "ifc_playlistloadercallback.h"
#include "ifc_playlist.h"
#include "ifc_playlistdirectorycallback.h"
#include <strsafe.h>

PlaylistManager playlistManager;

struct LoaderPair
{
	ifc_playlistloader *loader;
	svc_playlisthandler *handler;
};

static LoaderPair CreateLoader(const wchar_t *filename)
{
	LoaderPair ret = {0, 0};
	int n = 0;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			if (handler->SupportedFilename(filename) == SVC_PLAYLISTHANDLER_SUCCESS)
			{
				ret.loader = handler->CreateLoader(filename);
				ret.handler = handler;
				break;
			}
			else
			{
				sf->releaseInterface(handler);
			}
		}
	}

	// TODO: sniff file if no one claims it
	return ret;
}

void DestroyLoader(LoaderPair &loader)
{
	loader.handler->ReleaseLoader(loader.loader);
}

// a simple loader...
int PlaylistManager::Load(const wchar_t *filename, ifc_playlistloadercallback *playlist)
{
	LoaderPair loaderPair = CreateLoader(filename);
	ifc_playlistloader *loader = loaderPair.loader;

	if (!loader)
	{
		return PLAYLISTMANAGER_LOAD_NO_LOADER; // failed to find a loader
	}

	// TODO: make our own ifc_playlistloadercallback, so we can handle nested playlists
	int res = loader->Load(filename, playlist);
	DestroyLoader(loaderPair);

	if (res != IFC_PLAYLISTLOADER_SUCCESS) // TODO: switch on the error code and return a more specific error
		return PLAYLISTMANAGER_LOAD_LOADER_OPEN_FAILED;

	return PLAYLISTMANAGER_SUCCESS;
}

int PlaylistManager::LoadAs(const wchar_t *filename, const wchar_t *ext, ifc_playlistloadercallback *playlist)
{
	LoaderPair loaderPair = CreateLoader(ext);
	ifc_playlistloader *loader = loaderPair.loader;

	if (!loader)
	{
		return PLAYLISTMANAGER_LOAD_NO_LOADER; // failed to find a loader
	}

	// TODO: make our own ifc_playlistloadercallback, so we can handle nested playlists
	int res = loader->Load(filename, playlist);
	DestroyLoader(loaderPair);

	if (res != IFC_PLAYLISTLOADER_SUCCESS) // TODO: switch on the error code and return a more specific error
		return PLAYLISTMANAGER_LOAD_LOADER_OPEN_FAILED;

	return PLAYLISTMANAGER_SUCCESS;
}

static void MakeRelativePathName(const wchar_t *filename, wchar_t *outFile, size_t cch, const wchar_t *path)
{
	wchar_t outPath[MAX_PATH];

	int common = PathCommonPrefixW(path, filename, outPath);
	if (common && common == lstrlenW(path))
	{
		PathAddBackslashW(outPath);
		const wchar_t *p = filename + lstrlenW(outPath);
		lstrcpynW(outFile, p, cch);
	}
	else if (!PathIsUNCW(filename) && PathIsSameRootW(filename, path))
	{
		if (outFile[1] == ':')
			lstrcpynW(outFile, filename+2, cch);
	}
}

static void PlayList_makerelative(const wchar_t *base, wchar_t *filename, size_t cch)
{
	MakeRelativePathName(filename, filename, cch, base);
}

int PlaylistManager::Save(const wchar_t *filename, ifc_playlist *playlist)
{
	const wchar_t *ext = PathFindExtensionW(filename);
	PlaylistWriter *writer = 0;
	if (!lstrcmpiW(ext, L".M3U"))
		writer = new M3UWriter;
	else if (!lstrcmpiW(ext, L".M3U8"))
		writer = new M3U8Writer;
	else if (!lstrcmpiW(ext, L".PLS"))
		writer = new PLSWriter;
	else if (!lstrcmpiW(ext, L".B4S"))
		writer = new B4SWriter;
	else return PLAYLISTMANAGER_FAILED;

	wchar_t base[MAX_PATH];
	StringCchCopyW(base, MAX_PATH, filename);
	PathRemoveFileSpecW(base);
	PathRemoveBackslashW(base);

	if (!writer->Open(filename))
	{
		delete writer;
		return PLAYLISTMANAGER_FAILED;
	}

	size_t numItems = playlist->GetNumItems();
	wchar_t itemname[1024] = {0};
	wchar_t title[400] = {0}; // TODO: length???
	wchar_t cloud_info[512] = {0};
	int length;
	for (size_t i = 0; i != numItems; i++)
	{
		if (playlist->GetItem(i, itemname, 1024))
		{
			PlayList_makerelative(base, itemname, 1024);

			// this is used to preserve 'cloud' specific data in playlists
			// and should only get a response from a cloud-based ml_playlist
			if (playlist->GetItemExtendedInfo(i, L"cloud", cloud_info, 512))
			{
				writer->Write(cloud_info);
			}

			if (playlist->GetItemTitle(i, title, 400))
			{
				length = playlist->GetItemLengthMilliseconds(i);
				writer->Write(itemname, title, length / 1000);
			}
			else
				writer->Write(itemname);
		}
	}
	writer->Close();
	delete writer;
	return PLAYLISTMANAGER_SUCCESS;
}

size_t PlaylistManager::Copy(const wchar_t *destFn, const wchar_t *srcFn)
{
	Playlist copy;

	Load(srcFn, &copy);
	Save(destFn, &copy);

	return copy.GetNumItems();
}

size_t PlaylistManager::CountItems(const wchar_t *filename)
{
	LoaderPair loaderPair = CreateLoader(filename);
	ifc_playlistloader *loader = loaderPair.loader;

	if (!loader)
		return 0;

	PlaylistCounter counter;

	int res = loader->Load(filename, &counter);

	DestroyLoader(loaderPair);
	return counter.count;
}

void PlaylistManager::Reverse(ifc_playlist *playlist)
{
	if (playlist->Reverse() == PLAYLIST_UNIMPLEMENTED)
	{
		// TODO: do it the hard way
	}
}

void PlaylistManager::Randomize(ifc_playlist *playlist)
{
	if (playlist->Randomize(warand) == PLAYLIST_UNIMPLEMENTED)
	{
		// TODO: do it the hard way
	}
}

int PlaylistManager::LoadFromDialog(const wchar_t *fns, ifc_playlistloadercallback *playlist)
{
	wchar_t buf[MAX_PATH];
	const wchar_t *path = fns;
	fns += lstrlenW(fns) + 1;

	while (*fns)
	{
		if (*path)
			PathCombineW(buf, path, fns);
		else
			StringCchCopyW(buf, MAX_PATH, fns);

		if (Load(buf, playlist) != PLAYLISTMANAGER_SUCCESS)
		{
			if (playlist->OnFile(buf, 0, -1, 0) != ifc_playlistloadercallback::LOAD_CONTINUE)
				return PLAYLIST_SUCCESS;
		}

		fns += wcslen(fns) + 1;
	}
	return PLAYLIST_SUCCESS;
}

int PlaylistManager::LoadFromANSIDialog(const char *fns, ifc_playlistloadercallback *playlist)
{
	char buf[MAX_PATH];
	const char *path = fns;
	fns += lstrlenA(fns) + 1;

	while (*fns)
	{
		if (*path)
			PathCombineA(buf, path, fns);
		else
			lstrcpynA(buf, fns, MAX_PATH);

		AutoWide wideFn(buf);
		if (Load(wideFn, playlist) != PLAYLISTMANAGER_SUCCESS)
		{
			if (playlist->OnFile(wideFn, 0, -1, 0) !=  ifc_playlistloadercallback::LOAD_CONTINUE)
				return PLAYLIST_SUCCESS;
		}

		fns += lstrlenA(fns) + 1;
	}
	return PLAYLIST_SUCCESS;
}

int PlaylistManager::GetLengthMilliseconds(const wchar_t *filename)
{
	LoaderPair loaderPair = CreateLoader(filename);
	ifc_playlistloader *loader = loaderPair.loader;

	if (!loader)
		return 0;

	PlaylistCounter counter;
	loader->Load(filename, &counter);
	DestroyLoader(loaderPair);
	return (int)counter.length;
}

uint64_t PlaylistManager::GetLongLengthMilliseconds(const wchar_t *filename)
{
	LoaderPair loaderPair = CreateLoader(filename);
	ifc_playlistloader *loader = loaderPair.loader;

	if (!loader)
		return 0;

	PlaylistCounter counter;
	loader->Load(filename, &counter);
	DestroyLoader(loaderPair);
	return counter.length;
}

class NoRecurseCallback : public ifc_playlistdirectorycallback
{
public:
	NoRecurseCallback(ifc_playlistdirectorycallback *_callback) : callback(_callback) {}
	bool ShouldRecurse(const wchar_t *path) { return false;}
	bool ShouldLoad(const wchar_t *filename) { return callback->ShouldLoad(filename);}
	ifc_playlistdirectorycallback *callback;
protected:
	RECVS_DISPATCH;
};

#define CBCLASS NoRecurseCallback
START_DISPATCH;
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDRECURSE, ShouldRecurse)
CB(IFC_PLAYLISTDIRECTORYCALLBACK_SHOULDLOAD, ShouldLoad)
END_DISPATCH;
#undef CBCLASS

void PlaylistManager::LoadDirectory(const wchar_t *directory, ifc_playlistloadercallback *callback, ifc_playlistdirectorycallback *dirCallback)
{
	WIN32_FIND_DATAW found;

	wchar_t filespec[MAX_PATH];
	PathCombineW(filespec, directory, L"*.*");

	HANDLE i = FindFirstFileW(filespec, &found);
	if (i != INVALID_HANDLE_VALUE)
	{
		do
		{
			// if it's another folder, then we might want to recurse into it
			if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) // if it's a directory
			        && wcscmp(found.cFileName, L".") && wcscmp(found.cFileName, L"..") // but not . or ..
			        && (!dirCallback || dirCallback->ShouldRecurse(found.cFileName))) // and we're allowed to recurse
			{
				if (PathCombineW(filespec, directory, found.cFileName))
				{
					LoadDirectory(filespec, callback, dirCallback);
				}
			}

			if (!(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				const wchar_t *ext = PathFindExtensionW(found.cFileName);
				if (ext[0])
				{
					if (!_wcsicmp(ext, L".lnk"))
					{
						wchar_t thisf[MAX_PATH];
						wchar_t temp2[MAX_PATH];
						PathCombineW(temp2, directory, found.cFileName);
						if (ResolveShortCut(NULL, temp2, thisf) && GetLongPathNameW(thisf, temp2, MAX_PATH) && lstrcmpiW(temp2, directory))
						{
							HANDLE h2;
							WIN32_FIND_DATAW d2;
							if (IsUrl(temp2) && (!dirCallback || dirCallback->ShouldLoad(temp2)))
							{
								if (callback->OnFile(temp2, 0, -1, 0) != ifc_playlistloadercallback::LOAD_CONTINUE)
									break;
							}
							else
							{
								h2 = FindFirstFileW(temp2, &d2);
								if (h2 != INVALID_HANDLE_VALUE)
								{
									if (!(d2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
									{
										if (!dirCallback || dirCallback->ShouldLoad(temp2))
										{
											if (callback->OnFile(temp2, 0, -1, 0) != ifc_playlistloadercallback::LOAD_CONTINUE)
											{
												FindClose(h2);
												break;
											}
										}
									}
									else
									{
										// recursively load a shortcut w/o fear of infinite recursion
										NoRecurseCallback noRecurse(dirCallback);
										LoadDirectory(temp2, callback, &noRecurse);
									}
									FindClose(h2);
								}
							}
						}
					}
					else // !shortcut
					{
						
						if (PathCombineW(filespec, directory, found.cFileName) &&
							(!dirCallback || dirCallback->ShouldLoad(filespec)))
						{
							if (callback->OnFile(filespec, 0, -1, 0) != ifc_playlistloadercallback::LOAD_CONTINUE)
								break;
						}
					}
				}
			}
		}
		while (FindNextFileW(i, &found));
		FindClose(i);
	}
}

bool PlaylistManager::CanLoad(const wchar_t *filename)
{
	int n = 0;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			if (handler->SupportedFilename(filename) == SVC_PLAYLISTHANDLER_SUCCESS)
			{
				sf->releaseInterface(handler);
				return true;
			}
			else
			{
				sf->releaseInterface(handler);
			}
		}
	}
	return false;
}

void PlaylistManager::GetExtensionList(wchar_t *extensionList, size_t extensionListCch)
{
	extensionList[0] = 0;

	bool first = true;

	int n = 0, extListCch = extensionListCch;
	wchar_t *extList = extensionList;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			const wchar_t *ext = 0;
			int k = 0;
			while (ext = handler->EnumerateExtensions(k++))
			{
				if (first)
					StringCchCatExW(extensionList, extensionListCch, L"*.", &extensionList, &extensionListCch, 0);
				else
					StringCchCatExW(extensionList, extensionListCch, L";*.", &extensionList, &extensionListCch, 0);

				first = false;

				StringCchCatExW(extensionList, extensionListCch, ext, &extensionList, &extensionListCch, 0);
			}
			sf->releaseInterface(handler);
		}
	}
	CharUpperBuffW(extList, extListCch);
}

void PlaylistManager::GetFilterList(wchar_t *extensionList, size_t extensionListCch)
{
	extensionListCch--; // this needs to be DOUBLE null terminated, so we'll make sure there's room

	StringCchCopyExW(extensionList, extensionListCch, WASABI_API_LNGSTRINGW(IDS_ALL_PLAYLIST_TYPES), &extensionList, &extensionListCch, 0);
	extensionListCch--;
	extensionList++;

	GetExtensionList(extensionList, extensionListCch);

	extensionListCch -= (wcslen(extensionList) + 1);
	extensionList += wcslen(extensionList) + 1;

	int n = 0;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			const wchar_t *name = handler->GetName();
			if (!name)
				name = WASABI_API_LNGSTRINGW(IDS_PLAYLIST);

			StringCchCopyExW(extensionList, extensionListCch, name, &extensionList, &extensionListCch, 0);
			extensionList++;
			extensionListCch--;

			bool first = true;
			const wchar_t *ext = 0;
			int k = 0;
			while (ext = handler->EnumerateExtensions(k++))
			{
				if (first)
					StringCchCopyExW(extensionList, extensionListCch, L"*.", &extensionList, &extensionListCch, 0);
				else
					StringCchCatExW(extensionList, extensionListCch, L";*.", &extensionList, &extensionListCch, 0);

				first = false;

				StringCchCatExW(extensionList, extensionListCch, ext, &extensionList, &extensionListCch, 0);
			}
			extensionList++;
			extensionListCch--;

			sf->releaseInterface(handler);
		}
	}

	extensionList[0] = 0; // ok because we reserved the room for it above
}

const wchar_t *PlaylistManager::EnumExtensions(size_t num)
{
	int n = 0;
	int total = 0;
	waServiceFactory *sf = 0;
	while (sf = WASABI_API_SVC->service_enumService(WaSvc::PLAYLISTHANDLER, n++))
	{
		svc_playlisthandler * handler = static_cast<svc_playlisthandler *>(sf->getInterface());
		if (handler)
		{
			const wchar_t *ext = 0;
			int k = 0;
			while (ext = handler->EnumerateExtensions(k++))
			{
				if (total++ == num)
					return ext;
			}

			sf->releaseInterface(handler);
		}
	}

	return 0;
}

#define CBCLASS PlaylistManager
START_DISPATCH;
CB(API_PLAYLISTMANAGER_LOAD, Load)
CB(API_PLAYLISTMANAGER_LOADAS, LoadAs)
CB(API_PLAYLISTMANAGER_LOADNULLDELIMITED, LoadFromDialog)
CB(API_PLAYLISTMANAGER_LOADNULLDELIMITED_ANSI, LoadFromANSIDialog)
CB(API_PLAYLISTMANAGER_SAVE, Save)
CB(API_PLAYLISTMANAGER_COPY, Copy)
CB(API_PLAYLISTMANAGER_COUNT, CountItems)
CB(API_PLAYLISTMANAGER_GETLENGTH, GetLengthMilliseconds)
CB(API_PLAYLISTMANAGER_GETLONGLENGTH, GetLongLengthMilliseconds)
VCB(API_PLAYLISTMANAGER_RANDOMIZE, Randomize)
VCB(API_PLAYLISTMANAGER_REVERSE, Reverse)
VCB(API_PLAYLISTMANAGER_LOADDIRECTORY, LoadDirectory)
CB(API_PLAYLISTMANAGER_CANLOAD, CanLoad)
VCB(API_PLAYLISTMANAGER_GETEXTENSIONLIST, GetExtensionList)
VCB(API_PLAYLISTMANAGER_GETFILTERLIST, GetFilterList)
CB(API_PLAYLISTMANAGER_ENUMEXTENSION, EnumExtensions)
END_DISPATCH;
#undef CBCLASS