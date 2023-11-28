#include "main.h"
#include "pl_entry.h"
#include "plstring.h"
#include <wchar.h>
#include "../nu/strsafe.h"

pl_entry::pl_entry(const wchar_t *fn, const wchar_t *ft, int len)
	: cached(false), filename(0), filetitle(0), mediahash(0),
	  metahash(0), cloud_id(0), cloud_status(0), cloud_devices(0)
{
	SetFilename(fn);
	SetTitle(ft);
	SetLengthMilliseconds(len);
}

pl_entry::pl_entry(const wchar_t *fn, const wchar_t *ft, int len, int size)
	: cached(false), filename(0), filetitle(0), mediahash(0),
	  metahash(0), cloud_id(0), cloud_status(0), cloud_devices(0)
{
	SetFilename(fn);
	SetTitle(ft);
	SetLengthMilliseconds(len);
	SetSizeBytes(size);
}

pl_entry::pl_entry(const wchar_t *fn, const wchar_t *ft, int len, ifc_plentryinfo *info)
	: cached(false), filename(0), filetitle(0), mediahash(0),
	  metahash(0), cloud_id(0), cloud_status(0), cloud_devices(0)
{
	SetFilename(fn);
	SetTitle(ft);
	SetLengthMilliseconds(len);
	if (info)
	{
		SetMediahash(info->GetExtendedInfo(L"mediahash"));
		SetMetahash(info->GetExtendedInfo(L"metahash"));
		SetCloudID(info->GetExtendedInfo(L"cloud_id"));
		SetCloudStatus(info->GetExtendedInfo(L"cloud_status"));
		SetCloudDevices(info->GetExtendedInfo(L"cloud_devices"));
	}
}

pl_entry::pl_entry(const wchar_t *fn, const wchar_t *ft, int len,
				   const wchar_t *mediahash, const wchar_t *metahash,
				   const wchar_t *cloud_id, const wchar_t *cloud_status,
				   const wchar_t *cloud_devices)
	: cached(false), filename(0), filetitle(0), mediahash(0),
	  metahash(0), cloud_id(0), cloud_status(0), cloud_devices(0)
{
	SetFilename(fn);
	SetTitle(ft);
	SetLengthMilliseconds(len);
	SetMediahash(mediahash);
	SetMetahash(metahash);
	SetCloudID(cloud_id);
	SetCloudStatus(cloud_status);
	SetCloudDevices(cloud_devices);
}

pl_entry::~pl_entry()
{
	plstring_release(filename);
	plstring_release(filetitle);
	plstring_release(mediahash);
	plstring_release(metahash);
	plstring_release(cloud_id);
	plstring_release(cloud_status);
	plstring_release(cloud_devices);
}

size_t pl_entry::GetFilename(wchar_t *filename, size_t filenameCch)
{
	if (!this->filename)
		return 0;

	if (!filename)
		return wcslen(this->filename);

	if (!this->filename[0]) 
		return 0;

	StringCchCopyW(filename, filenameCch, this->filename);
	return 1;
}

size_t pl_entry::GetTitle(wchar_t *title, size_t titleCch)
{
	if (!this->filetitle)
		return 0;

	if (!title)
		return wcslen(this->filetitle);

	if (!this->filetitle[0]) 
		return 0;

	StringCchCopyW(title, titleCch, this->filetitle);
	return 1;
}

int pl_entry::GetLengthInMilliseconds()
{
	return this->length;
}

int pl_entry::GetSizeInBytes()
{
	return this->size;
}

size_t pl_entry::GetExtendedInfo(const wchar_t *metadata, wchar_t *info, size_t infoCch)
{
	if (cloud_id)
	{
		if (!_wcsnicmp(L"mediahash", metadata, 9) && mediahash)
		{
			lstrcpynW(info, mediahash, infoCch);
			return 1;
		}
		else if (!_wcsnicmp(L"metahash", metadata, 8) && metahash)
		{
			lstrcpynW(info, metahash, infoCch);
			return 1;
		}
		else if (!_wcsnicmp(L"cloud_id", metadata, 8) && cloud_id)
		{
			lstrcpynW(info, cloud_id, infoCch);
			return 1;
		}
		else if (!_wcsnicmp(L"cloud_status", metadata, 12) && cloud_status)
		{
			lstrcpynW(info, cloud_status, infoCch);
			return 1;
		}
		else if (!_wcsnicmp(L"cloud_devices", metadata, 13) && cloud_devices)
		{
			lstrcpynW(info, cloud_devices, infoCch);
			return 1;
		}
		else if (!_wcsnicmp(metadata, L"cloud", 5))
		{
			if (_wtoi(cloud_id) > 0)
			{
				StringCchPrintfW(info, infoCch, L"#EXT-X-NS-CLOUD:mediahash=%s,metahash=%s,cloud_id=%s,cloud_status=%s,cloud_devices=%s",
								 (mediahash && *mediahash ? mediahash : L""),
								 (metahash && *metahash ? metahash : L""), cloud_id,
								 (cloud_status && *cloud_status ? cloud_status : L""),
								 (cloud_devices && *cloud_devices ? cloud_devices : L""));
				return 1;
			}
		}
	}
	return 0;
}

void pl_entry::SetFilename(const wchar_t *filename)
{
	plstring_release(this->filename);

	if (filename && filename[0])
		this->filename = plstring_wcsdup(filename);
	else
		this->filename = 0;
}

void pl_entry::SetTitle(const wchar_t *title)
{
	plstring_release(this->filetitle);

	if (title && title[0])
	{
		this->filetitle = plstring_wcsdup(title);
		cached = true;
	}
	else
		this->filetitle = 0;
}

void pl_entry::SetLengthMilliseconds(int length)
{
	if (length <= 0)
		this->length = -1000;
	else
		this->length = length;
}

void pl_entry::SetMediahash(const wchar_t *mediahash)
{
	plstring_release(this->mediahash);

	if (mediahash && mediahash[0])
		this->mediahash = plstring_wcsdup(mediahash);
	else
		this->mediahash = 0;
}

void pl_entry::SetSizeBytes(int size)
{
	if (size <= 0)
		this->size = 0;
	else
		this->size = size;
}

void pl_entry::SetMetahash(const wchar_t *metahash)
{
	plstring_release(this->metahash);

	if (metahash && metahash[0])
		this->metahash = plstring_wcsdup(metahash);
	else
		this->metahash = 0;
}

void pl_entry::SetCloudID(const wchar_t *cloud_id)
{
	plstring_release(this->cloud_id);

	if (cloud_id && cloud_id[0] && _wtoi(cloud_id) > 0)
		this->cloud_id = plstring_wcsdup(cloud_id);
	else
		this->cloud_id = 0;
}

void pl_entry::SetCloudStatus(const wchar_t *cloud_status)
{
	plstring_release(this->cloud_status);

	if (cloud_status && cloud_status[0] && _wtoi(cloud_status) >= 0)
		this->cloud_status = plstring_wcsdup(cloud_status);
	else
		this->cloud_status = 0;
}

void pl_entry::SetCloudDevices(const wchar_t *cloud_devices)
{
	plstring_release(this->cloud_devices);

	if (cloud_devices && cloud_devices[0])
		this->cloud_devices = plstring_wcsdup(cloud_devices);
	else
		this->cloud_devices = 0;
}

#define CBCLASS pl_entry
START_DISPATCH;
CB(IFC_PLAYLISTENTRY_GETFILENAME, GetFilename)
CB(IFC_PLAYLISTENTRY_GETTITLE, GetTitle)
CB(IFC_PLAYLISTENTRY_GETLENGTHMS, GetLengthInMilliseconds)
CB(IFC_PLAYLISTENTRY_GETEXTENDEDINFO, GetExtendedInfo)
END_DISPATCH;
#undef CBCLASS