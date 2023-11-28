#ifndef NULLSOFT_ML_PLAYLISTS_PL_ENTRY_H
#define NULLSOFT_ML_PLAYLISTS_PL_ENTRY_H

#include "ifc_playlistentry.h"
#include "ifc_plentryinfo.h"
#include <windows.h>
class pl_entry : public ifc_playlistentry
{
public:
	size_t GetFilename(wchar_t *filename, size_t filenameCch);
	size_t GetTitle(wchar_t *title, size_t titleCch);
	int GetLengthInMilliseconds();
	int GetSizeInBytes();
	size_t GetExtendedInfo(const wchar_t *metadata, wchar_t *info, size_t infoCch);

	void SetFilename(const wchar_t *filename);
	void SetTitle(const wchar_t *title);
	void SetLengthMilliseconds(int length);
	void SetSizeBytes(int size);
	void SetMediahash(const wchar_t *mediahash);
	void SetMetahash(const wchar_t *metahash);
	void SetCloudID(const wchar_t *cloud_id);
	void SetCloudStatus(const wchar_t *cloud_status);
	void SetCloudDevices(const wchar_t *cloud_devices);

public:
	pl_entry() 
	{
		filename = 0;
		filetitle = 0;
		mediahash = 0;
		metahash = 0;
		cloud_id = 0;
		cloud_status = 0;
		cloud_devices = 0;
		length = -1;
		cached = false;
	}
	~pl_entry();
	pl_entry(const wchar_t *fn, const wchar_t *ft, int len);
	pl_entry(const wchar_t *fn, const wchar_t *ft, int len, int size);
	pl_entry(const wchar_t *fn, const wchar_t *ft, int len, ifc_plentryinfo *info);
	pl_entry(const wchar_t *fn, const wchar_t *ft, int len,
			 const wchar_t *mediahash, const wchar_t *metahash,
			 const wchar_t *cloud_id, const wchar_t *cloud_status,
			 const wchar_t *cloud_devices);

	wchar_t *filename;
	wchar_t *filetitle;
	wchar_t *mediahash;
	wchar_t *metahash;
	wchar_t *cloud_id;
	wchar_t *cloud_status;
	wchar_t *cloud_devices;
	int length;
	int size;
	bool cached;

protected:
	RECVS_DISPATCH;
};

#endif