#include "main.h"
#include "api.h"
#include "config.h"

static bool IsVideo(const wchar_t *filename, const itemRecordW *record)
{
	if (record)
	{
			return (record->type == 1);
	}
	else
	{
		wchar_t metadata[64]=L"";
		if (AGAVE_API_METADATA->GetExtendedFileInfo(filename, L"type", metadata, 64) && metadata[0] == L'1')
			return true;

		return false;		
	}
}

static bool FileInDirectory(const wchar_t *file, const wchar_t *directory, size_t directory_len)
{
	if (!_wcsnicmp(directory, file, directory_len) && (file[directory_len] == L'\\' || file[directory_len] == L'/'))
	{
		return true;
	}
	else
	{
		return false;
	}
}

static bool AllowedDirectory(const wchar_t *filename)
{
	wchar_t *directories_itr = config_directories;
	while (directories_itr && *directories_itr)
	{
		size_t directory_len = 0;
		wchar_t *end_delimiter = wcschr(directories_itr, L'|');
		if (end_delimiter)
		{
			directory_len = end_delimiter - directories_itr;
			end_delimiter++;
		}
		else
		{
			directory_len = wcslen(directories_itr);
		}
		if (FileInDirectory(filename, directories_itr, directory_len))
			return true;
		directories_itr = end_delimiter;
	}
	return false;
}



bool AllowedFilename(const wchar_t *filename)
{
	//verify filename and filepath against a whitelist/blacklist (e.g. in media library, in certain folders, etc)

	itemRecordW *record=0;

	// query the media library for the track if we need to
	if (AGAVE_API_MLDB && (config_allow_mode == CONFIG_MODE_MEDIA_LIBRARY || config_video == 0))
		record = AGAVE_API_MLDB->GetFile(filename);

	// check if it's a video file
	if (config_video == 0 && IsVideo(filename, record))
	{
			if (record)
				AGAVE_API_MLDB->FreeRecord(record);
			return false;
	}
	
	switch(config_allow_mode)
	{
	case CONFIG_MODE_ALL:
		if (record)
			AGAVE_API_MLDB->FreeRecord(record);
		return true;
		
	case CONFIG_MODE_MEDIA_LIBRARY:
		if (record)
		{
			AGAVE_API_MLDB->FreeRecord(record);
			return true;
		}
		else
			return false;
	case CONFIG_MODE_DIRECTORIES:
		if (record)
			AGAVE_API_MLDB->FreeRecord(record);
		return AllowedDirectory(filename);
	}

	// shouldn't get here
	return false;
}
