#pragma once
#include <bfc/platform/types.h>

typedef uint32_t CueTime; // packed MSF format

// this magic value indicates that a file is played to the end
// e.g. from CueSheet_GetTrackPlayInfo
const CueTime CueSheet_EOF = 0xFFFFFFFFU;

enum
{
	CUESHEET_OK=0,
	CUESHEET_EMPTY_LINE, // an empty line occurred in the CUE sheet
	CUESHEET_BUFFER_SIZE, // CUE sheet contained a line too long for the internal buffer (CueSheet_Open) or the buffer you passed was too small
	CUESHEET_INVALID_TRACK_NUMBER, // TRACK number was invalid (CueSheet_Open) or track number not found
	CUESHEET_MALFORMED, // CUE sheet syntax error or unknown keyword
	CUESHEET_DUPLICATE_INDEX, // two INDEXs with the same number
	CUESHEET_INVALID_INDEX_TIME, // and INDEX's time format was invalid
	CUESHEET_INVALID_INDEX, // an INDEX ## number was invalid
	CUESHEET_UNSUPPORTED, // CUE sheet syntax was understood, but not supported (e.g. POSTGAP)
	CUESHEET_BAD_OBJECT, // you passed in a NULL sheet or something
  CUESHEET_ERROR, // an error occured that was generally unexpected or difficult to categorize
	CUESHEET_ENCODING, // unsupported UTF-16 text encoding
	CUESHEET_FILE_NOT_FOUND,
};

typedef void *CueSheet;

#ifdef WIN32
int CueSheet_Open(const wchar_t *filename, CueSheet **sheet);
#else
int CueSheet_Open(const char *filename, CueSheet **sheet);
#endif

/* filenames are stored as 8bit text in the CUE sheet, so they are returned as such here
interpreting the text as ASCII or UTF-8 or local code page is an exercise left for the reader 
filename pointer persists until you call CueSheet_Close.  Make a copy if you need it longer */
int CueSheet_GetTrackPlayInfo(const CueSheet *sheet, uint8_t trackNumber, const char ** filename, CueTime *start, CueTime *end);
/* Get the first track number and the total number of tracks
The CUE sheet specification (and I guess the MMC spec) allows for the first track to anywhere from 1-99
although all tracks after must be sequential.
numTracks value is independent of firstTrack, e.g. if the CD tracks are number 8 through 10, then
firstTrack=8 and numTracks=3 
If you pass a non-NULL firstTrack pointer and there are no tracks in the CUE sheet, CUESHEET_INVALID_TRACK_NUMBER is returned
*/
int CueSheet_GetNumberOfTracks(const CueSheet *sheet, uint8_t *firstTrack, uint8_t *numTracks);

void CueSheet_Close(CueSheet *sheet);
