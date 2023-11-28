#include "nscue.h"
#include "../nu/PtrMap.h"
#include "../nu/Map.h"
#ifdef WIN32
#include "../nu/ns_wc.h"
#endif
#include "../nu/PtrList.h"
#include "../nu/strsafe.h"
#include <stdio.h>
#include <ctype.h>
#define CUE_FILESIZE 1024 // TODO: replace with _PATH_MAX or whatever
struct Internal_CueTrack
{
	Internal_CueTrack();
	char type[256];
	char title[2048];
	char performer[2048];
	char ISRC[13];
	char songwriter[2048];
	uint8_t trackNumber;
	CueTime pregap;
	// using int8_t as key for indices as an implementation detail
	// since we use -1 as "highest index" when indices is empty
	nu::Map<int8_t, CueTime> indices;
	int GetIndexTime(int8_t index, CueTime *t) const;
};
int Internal_CueTrack::GetIndexTime(int8_t index, CueTime *t) const
{
	nu::Map<int8_t, CueTime>::iterator itr = indices.find(index);
	if (itr == indices.end())
		return CUESHEET_INVALID_INDEX;

	*t = itr->second;
	return CUESHEET_OK;
}
Internal_CueTrack::Internal_CueTrack()
{
	type[0]=0;
	title[0]=0;
	performer[0]=0;
	ISRC[0]=0;
	songwriter[0]=0;
	trackNumber=0;
	pregap = 0;
}
struct Internal_File
{
	Internal_File()
	{
		file[0]=0;
	}
	char file[CUE_FILESIZE];
	uint8_t track; 
	int8_t index; 
	/* this filename is to be used for all tracks with 
	trackNumber > InternalFile::track 
	|| (trackNumber == InternalFile::track && index > InternalFile::index)
	*/
};
static bool Internal_CueSheet_FileValidForTrack(const Internal_File *file, const Internal_CueTrack *track, int8_t index)
{
	return track->trackNumber > file->track || (track->trackNumber == file->track && index > file->index);
}
struct Internal_CueSheet
{
	Internal_CueSheet()
	{
		title[0]=0;
		performer[0]=0;
		catalog[0]=0;
		songwriter[0]=0;
	}
	char title[2048];
	char performer[2048];
	char catalog[14];
	char songwriter[2048];
	nu::PtrList<Internal_CueTrack> tracks;
	nu::PtrList<Internal_File> files;
	int GetTrack(uint8_t trackNumber, Internal_CueTrack **track) const;
	int GetFileForTrack(const Internal_CueTrack *track, int8_t index, Internal_File **file) const;
	int GetFirstTrackNumber(uint8_t *trackNumber) const;
};
int Internal_CueSheet::GetFileForTrack(const Internal_CueTrack *track, int8_t index, Internal_File **file) const
{
	if (!track || !file)
		return CUESHEET_BAD_OBJECT;
	for (nu::PtrList<Internal_File>::iterator itr=files.begin();itr!=files.end();itr++)
	{
		if (Internal_CueSheet_FileValidForTrack(*itr, track, index))
		{
			*file = *itr;
			return CUESHEET_OK;
		}
	}
	return CUESHEET_ERROR;
}
int Internal_CueSheet::GetFirstTrackNumber(uint8_t *trackNumber) const
{
	if (!trackNumber)
		return CUESHEET_BAD_OBJECT;

	if (tracks.empty())
		return CUESHEET_INVALID_TRACK_NUMBER;

	*trackNumber = tracks[0]->trackNumber;
	return CUESHEET_OK;
}

int Internal_CueSheet::GetTrack(uint8_t trackNumber, Internal_CueTrack **track) const
{
	if (!track)
		return CUESHEET_BAD_OBJECT;

	if (tracks.empty())
		return CUESHEET_INVALID_TRACK_NUMBER;
	// tracks don't have to start at 1, according to the specs (but they must be sequential after that)
	// so we'll offset from the first track number in the array
	uint8_t trackOffset = tracks[0]->trackNumber;
	if (trackNumber > trackOffset)
		return CUESHEET_INVALID_TRACK_NUMBER;

	trackNumber -= trackOffset;

	if (trackNumber >= tracks.size())
		return CUESHEET_INVALID_TRACK_NUMBER;

	*track = tracks[trackNumber];
	return CUESHEET_OK;
}
static const char *Internal_CueSheet_EatSpaces(const char *line)
{
	while (*line && (*line == ' ' || *line == '\t'))
		line++;
	return line;
}
static int Internal_CueSheet_ReadNextWord(const char *&line, char *str, size_t strcch)
{
	line = Internal_CueSheet_EatSpaces(line);
	if (line)
	{
		bool inQuotes = false;
		while (strcch)
		{
			if (*line == 0 // out of stuff to copy
				|| *line == '\r' || *line == '\n' // newline
				|| (*line == ' ' && !inQuotes)) // or we found a space
			{
				*str = 0;
				return CUESHEET_OK;
			}
			else if (*line == '\"') // don't copy quotes
			{
				inQuotes = !inQuotes; // but do toggle the quote flag (so we can ignore spaces)
			}
			else // safe character to copy
			{
				// benski> we don't really need g_utf8_find_next_char because space and quotes aren't
				// characters that would appear in the 2nd/3rd bytes of a UTF-8 sequence
				*str++=*line;
				strcch--;
			}
			// see above comment about g_utf8_find_next_char
			line++;
		}
		return CUESHEET_BUFFER_SIZE;
	}	
	else
		return CUESHEET_EMPTY_LINE;
}
static bool Internal_CueSheet_MFS_Valid(const char *mfs)
{
	return isdigit(mfs[0]) && isdigit(mfs[1]) && mfs[2]==':'
		&& isdigit(mfs[3]) && isdigit(mfs[4]) && mfs[5]==':'
		&& isdigit(mfs[6]) && isdigit(mfs[7]) && mfs[8]==0;
}
// this function assumes that you've already validated the string
static uint32_t Internal_CueSheet_MFS_StringToUInt32(const char *mfs)
{
	uint32_t val = 0;
	uint8_t *valptr = (uint8_t *)&val;
	valptr[1] = (uint8_t)strtoul(mfs, 0, 10);
	valptr[2] = (uint8_t)strtoul(mfs+3, 0, 10);
	valptr[3] = (uint8_t)strtoul(mfs+6, 0, 10);
	return val;
}

static int8_t Internal_CueSheet_LowestIndex(const Internal_CueTrack *track)
{
	if (!track)
		return -1;
	if (track->indices.empty())
		return -1;
	int8_t max=-1;
	for (nu::Map<int8_t, CueTime>::iterator itr=track->indices.begin();itr!=track->indices.end();itr++)
	{
		if (itr->first > max)
			max = itr->first;
	}
	return max;
}
static int Internal_CueSheet_ParseTrack(const char *line, Internal_CueTrack *track)
{
	char command[256];
	int err = Internal_CueSheet_ReadNextWord(line, command, 256);
	if (err != CUESHEET_OK)
		return err;
	if (!strcmp(command, "TITLE"))
	{
		int err = Internal_CueSheet_ReadNextWord(line, track->title, sizeof(track->title));
		if (err != CUESHEET_OK)
			return err;
	}
	else if (!strcmp(command, "PERFORMER"))
	{
		int err = Internal_CueSheet_ReadNextWord(line, track->performer, sizeof(track->performer));
		if (err != CUESHEET_OK)
			return err;
	}
	else if (!strcmp(command, "REM"))
	{
		// do nothing
	}
	else if (!strcmp(command, "PREGAP"))
	{
		char mfs_str[9];
		int err = Internal_CueSheet_ReadNextWord(line, mfs_str, 9);
		if (err != CUESHEET_OK)
			return err;

		if (!Internal_CueSheet_MFS_Valid(mfs_str))
		{
			printf("Bad MFS string %s\n", mfs_str);
			return CUESHEET_INVALID_INDEX_TIME;
		}

		uint32_t mfs = Internal_CueSheet_MFS_StringToUInt32(mfs_str);
		track->pregap = mfs;

		return CUESHEET_OK;
		// TODO: ?
	}
	else if (!strcmp(command, "POSTGAP"))
	{
		return CUESHEET_UNSUPPORTED;
		// TODO: ?
	}
	else if (!strcmp(command, "FLAGS"))
	{
		// TODO: ?
	}
	else if (!strcmp(command, "ISRC"))
	{
		int err = Internal_CueSheet_ReadNextWord(line, track->ISRC, sizeof(track->ISRC));
		if (err != CUESHEET_OK)
			return err;
	}
	else if (!strcmp(command, "INDEX"))
	{
		char indexNumber[64];
		int err = Internal_CueSheet_ReadNextWord(line, indexNumber, 64);
		if (err != CUESHEET_OK)
			return err;
		uint32_t index_full = strtoul(indexNumber, 0, 10);
		if (index_full > 99)
		{
			return CUESHEET_INVALID_INDEX;
		}
		int8_t index = (int8_t)index_full;
		if (track->indices.find(index) != track->indices.end())
		{
			printf("Duplicate Index %u\n", index);
			return CUESHEET_DUPLICATE_INDEX;
		}
		char mfs_str[10];
		err = Internal_CueSheet_ReadNextWord(line, mfs_str, 10);
		if (err != CUESHEET_OK)
			return err;
		if (!Internal_CueSheet_MFS_Valid(mfs_str))
		{
			printf("Bad MFS string %s\n", mfs_str);
			return CUESHEET_INVALID_INDEX_TIME;
		}
		uint32_t mfs = Internal_CueSheet_MFS_StringToUInt32(mfs_str);
		track->indices[index] = mfs;
	}
	else if (command[0])
	{
		printf("Unknown keyword %s in TRACK\n", command);
		return CUESHEET_MALFORMED;
	}
	return CUESHEET_OK;
}

enum
{
	CUESHEET_ENCODING_8BIT,
	CUESHEET_ENCODING_UTF16LE,
	CUESHEET_ENCODING_UTF16BE,
	CUESHEET_ENCODING_UTF8,
};

static int Internal_CueSheet_GetEncoding(const char *linedata)
{
	const uint8_t *raw_data = (const uint8_t *)linedata;
if (raw_data[0] == 0xFF && raw_data[1] == 0xFE)
{
	return CUESHEET_ENCODING_UTF16LE;
}
if (raw_data[0] == 0xFE && raw_data[1] == 0xFF)
{
return CUESHEET_ENCODING_UTF16BE;
}
if (raw_data[0] == 0xEF && raw_data[1] == 0xBB && raw_data[2] == 0xBF)
{
return CUESHEET_ENCODING_UTF8;
}
return CUESHEET_ENCODING_8BIT;
}

#ifdef WIN32
static size_t Internal_CueSheet_UTF16Len(const char *data, size_t buffer_max_size)
{
	const wchar_t *utf16 = (const wchar_t *)data;
	size_t s=0;
	while (buffer_max_size && *utf16 && *utf16 != '\n' && *utf16 != '\r')
	{
		utf16++;
		s++;
		buffer_max_size-=2;
	}
	return s;
}
#endif

static int Internal_CueSheet_Open(FILE *f, Internal_CueSheet *sheet)
{
	Internal_CueTrack *curTrack = 0;
	int encoding = CUESHEET_ENCODING_8BIT;
	bool first_line=true;
	char BOM[3];
	// TODO: UTF8 BOM?
	size_t bom_read = fread(BOM, 1, 3, f);
	if (bom_read == 3)
	{
		encoding = Internal_CueSheet_GetEncoding(BOM);
		switch(encoding)
		{
		case CUESHEET_ENCODING_8BIT:
			fseek(f, 0, SEEK_SET);
				break;
		case CUESHEET_ENCODING_UTF16LE:
		case CUESHEET_ENCODING_UTF16BE:
			fseek(f, 2, SEEK_SET);
		}
	}
	else
		return CUESHEET_ERROR;
	
	char linedata[2048];
	while (1)
	{
		if (encoding == CUESHEET_ENCODING_UTF16LE)
		{
#ifdef WIN32
			wchar_t utf16[1024];
			wchar_t *ret = fgetws(utf16, 1024, f);
			if (!ret)
				break;

			WideCharToMultiByteSZ(CP_UTF8, 0, utf16, -1, linedata, sizeof(linedata), 0, 0);
#else
			return CUESHEET_ENCODING;
#endif
		}
	else if (encoding == CUESHEET_ENCODING_UTF16BE)
	{
		return CUESHEET_ENCODING;
	}
	else
	{
			char *ret = fgets(linedata, 2048, f);
			if (!ret)
				break;
		}
		const char *line = linedata;
		line = Internal_CueSheet_EatSpaces(line);
		if (line && *line)
		{
			char command[256];
			int err = Internal_CueSheet_ReadNextWord(line, command, 256);
			if (err != CUESHEET_OK)
			{
				delete curTrack;
				return err;
			}
			if (!strcmp(command, "FILE"))
			{
				Internal_File *file = new Internal_File;
				// filename
				int err = Internal_CueSheet_ReadNextWord(line, file->file, sizeof(file->file));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
				file->track = curTrack?curTrack->trackNumber:0;
				file->index = Internal_CueSheet_LowestIndex(curTrack);
				sheet->files.push_back(file);
			}
			else if (!strcmp(command, "TRACK"))
			{
				if (curTrack)
				{
					sheet->tracks.push_back(curTrack);
					curTrack = 0;
				}
				curTrack = new Internal_CueTrack;
				// track track #
				char trackNumber[256];
				int err = Internal_CueSheet_ReadNextWord(line, trackNumber, 256);
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
				uint32_t trackNumber_full = strtoul(trackNumber, 0, 10);
				if (trackNumber_full == 0 || trackNumber_full > 99)
				{
					delete curTrack;
					return CUESHEET_INVALID_TRACK_NUMBER;
				}
				curTrack->trackNumber=(uint8_t)trackNumber_full;
				// make sure the track # is valid
				if (!sheet->tracks.empty() && curTrack->trackNumber != sheet->tracks.back()->trackNumber + 1)
				{
					delete curTrack;
					return CUESHEET_INVALID_TRACK_NUMBER;
				}
				err = Internal_CueSheet_ReadNextWord(line, curTrack->type, sizeof(curTrack->type));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (curTrack)
			{
				// if we have an active track and command != "track",
				// pass linedata (not line) to CueSheet_ParseTrack
				int err = Internal_CueSheet_ParseTrack(linedata, curTrack);
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (!strcmp(command, "TITLE"))
			{
				// album name
				int err = Internal_CueSheet_ReadNextWord(line, sheet->title, sizeof(sheet->title));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (!strcmp(command, "CDTEXTFILE"))
			{
				// TODO: ?
			}
			else if (!strcmp(command, "CATALOG"))
			{
				// catalog number
				int err = Internal_CueSheet_ReadNextWord(line, sheet->catalog, sizeof(sheet->catalog));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (!strcmp(command, "PERFORMER"))
			{
				// artist
				int err = Internal_CueSheet_ReadNextWord(line, sheet->performer, sizeof(sheet->performer));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (!strcmp(command, "SONGWRITER"))
			{
				// artist
				int err = Internal_CueSheet_ReadNextWord(line, sheet->songwriter, sizeof(sheet->songwriter));
				if (err != CUESHEET_OK)
				{
					delete curTrack;
					return err;
				}
			}
			else if (!strcmp(command, "REM"))
			{
				// do nothing
				/* TODO: parse, e.g.
				REM GENRE Trance
				REM DATE 2004
				REM DISCID 8012B50A
				REM COMMENT "ExactAudioCopy v0.99pb3"
				*/
			}
			else if (command[0] == ';')
			{
				// Medieval CUE Splitter (www.medieval.it)
				// uses this as a comment character
				// so skip it
			}
			else
			{
				printf("Unknown keyword %s\n", command);
				delete curTrack;
				return CUESHEET_MALFORMED;
			}
		}
	}
	if (curTrack)
	{
		sheet->tracks.push_back(curTrack);
	}
	return CUESHEET_OK;
}
#ifdef WIN32
int CueSheet_Open(const wchar_t *filename, CueSheet **sheet)
#else
int CueSheet_Open(const char *filename, CueSheet **sheet)
#endif
{
#ifdef WIN32
	FILE *f = _wfopen(filename, L"rb");
#else
	FILE *f = fopen(filename, "r");
#endif
	if (!f)
		return CUESHEET_FILE_NOT_FOUND;
	Internal_CueSheet *new_sheet = new Internal_CueSheet;
	int err = Internal_CueSheet_Open(f, new_sheet);
	if (err != CUESHEET_OK)
	{
		*sheet = 0;
		delete new_sheet;
	}
	else
	{
		*sheet = (CueSheet *)new_sheet;
	}
	fclose(f);
	return err;
}
void CueSheet_Close(CueSheet *sheet)
{
	if (sheet)
	{
		Internal_CueSheet *new_sheet = (Internal_CueSheet *)sheet;
		delete sheet;
	}
}
int CueSheet_GetTrackPlayInfo(const CueSheet *sheet, uint8_t trackNumber, const char **filename, CueTime *start, CueTime *end)
{
	if (sheet)
	{
		Internal_CueSheet *sheetobj = (Internal_CueSheet *)sheet;
		Internal_CueTrack *track = 0;

		// lookup the track object
		int err = sheetobj->GetTrack(trackNumber, &track);
		if (err != CUESHEET_OK)
			return err;

		// find the corresponding file
		Internal_File *file = 0;
		err = sheetobj->GetFileForTrack(track, /*index=1*/1, &file);
		if (err != CUESHEET_OK)
			return err;

		if (filename)
		{
			*filename = file->file;
		}

		if (start)
		{
			err = track->GetIndexTime(/*index=*/1, start);
			if (err != CUESHEET_OK)
				return err;
		}

		// to find the end time we have to jump through a few hoops
		if (end)
		{
			// first, find the next track
			Internal_CueTrack *nexttrack = 0;
			err = sheetobj->GetTrack(trackNumber+1, &nexttrack);
			if (err == CUESHEET_INVALID_TRACK_NUMBER)
			{
				// last track, we're done!
				*end = CueSheet_EOF;
				return CUESHEET_OK;
			}
			else if (err != CUESHEET_OK)
				return err;

			// ok now we need to find the corresponding file for the next track
			// to see if it's the same file as ours
			Internal_File *nextfile = 0;
			err = sheetobj->GetFileForTrack(nexttrack, /*index=1*/1, &nextfile);
			if (err != CUESHEET_OK)
				return err;

			if (nextfile == file  // if they're the exact same object
				|| !strcmp(nextfile->file, file->file)) // or the filenames are the same (maybe some stupid program wrote the CUE sheet)
			{
				// end time is the start of the next track
				return track->GetIndexTime(/*index=*/1, end);
			}
			else
			{
				// different files, so just read to the end of the current file
				*end = CueSheet_EOF;
				return CUESHEET_OK;
			}        
		}
		return CUESHEET_OK;
	}
	else
		return CUESHEET_BAD_OBJECT;
}

int CueSheet_GetNumberOfTracks(const CueSheet *sheet, uint8_t *firstTrack, uint8_t *numTracks)
{
	if (!sheet)
		return CUESHEET_BAD_OBJECT;
	Internal_CueSheet *sheetobj = (Internal_CueSheet *)sheet;
	if (numTracks)   
		*numTracks = sheetobj->tracks.size();
	if (firstTrack)
	{
		int err = sheetobj->GetFirstTrackNumber(firstTrack);
		if (err != CUESHEET_OK)
			return err;
	}
	return CUESHEET_OK;
}
