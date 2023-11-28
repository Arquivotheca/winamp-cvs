#pragma once
#include "nx/nxapi.h"
#include <stdio.h> // for FILE
#include "nx/nxuri.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "foundation/error.h"
#include "nx/nxtime.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct nx_file_stat_s
	{
		nx_time_unix_64_t creation_time;
		nx_time_unix_64_t access_time;
		nx_time_unix_64_t modified_time;
		uint64_t file_size;
	} nx_file_stat_s, *nx_file_stat_t;

	typedef enum
	{
		nx_file_FILE_none = 0,
		nx_file_FILE_binary = (1 << 1),
		nx_file_FILE_read_text= (1 << 0),
		nx_file_FILE_read_binary=nx_file_FILE_read_text|nx_file_FILE_binary,
		nx_file_FILE_write_text=(1 << 2),
		nx_file_FILE_write_binary=nx_file_FILE_write_text|nx_file_FILE_binary,
		nx_file_FILE_update_text=(1 << 3),
		nx_file_FILE_update_binary=nx_file_FILE_update_text|nx_file_FILE_binary,
		nx_file_FILE_readwrite_text=(1 << 4),
		nx_file_FILE_readwrite_binary=nx_file_FILE_readwrite_text|nx_file_FILE_binary,

		nx_file_FILE_writable_mask = nx_file_FILE_write_text|nx_file_FILE_update_text|nx_file_FILE_readwrite_text,
	} nx_file_FILE_flags_t;

	static const int nx_file_O_BINARY=0;
	static const int nx_file_O_WRONLY=O_WRONLY;
	static const int nx_file_O_RDONLY=O_RDONLY;

	static FILE *NXFile_fopen(nx_uri_t filename, nx_file_FILE_flags_t flags)
	{
		if (flags == nx_file_FILE_read_binary)
		{
			return fopen(filename->string, "rb");
		}
		else if (flags == nx_file_FILE_write_binary)
		{
			return fopen(filename->string, "wb");
		}
		else if (flags == nx_file_FILE_update_binary)
		{
			return fopen(filename->string, "r+b");
		}
		else if (flags == nx_file_FILE_readwrite_binary)
		{
			return fopen(filename->string, "w+b");
		}
		return 0;
	}

	NX_API int NXFile_open(nx_uri_t filename, int flags);

	NX_API int NXFile_move(nx_uri_t destination, nx_uri_t source);
	NX_API int NXFile_unlink(nx_uri_t filename);
	NX_API int NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats);
	NX_API int NXFile_statFILE(FILE *f, nx_file_stat_t file_stats);
	NX_API int NXFile_fstat(int file_descriptor, nx_file_stat_t file_stats);

		/* --------------------------------------------------------------------------- */
	typedef struct nx_file_s { size_t dummy; } *nx_file_t;
	NX_API ns_error_t NXFileOpenFile(nx_file_t *out_file, nx_uri_t filename, nx_file_FILE_flags_t flags);
	NX_API nx_file_t NXFileRetain(nx_file_t f);
	NX_API void NXFileRelease(nx_file_t f);
	/* the implementation of this function will only return NErr_EndOfFile if 0 bytes were read. 
	   when *bytes_read < bytes_requested, it's likely that the file is at the end, but it will still return NErr_Success
	   until the next call */
	NX_API ns_error_t NXFileRead(nx_file_t f, void *buffer, size_t bytes_requested, size_t *bytes_read);
	NX_API ns_error_t NXFileWrite(nx_file_t f, const void *buffer, size_t bytes);
	NX_API ns_error_t NXFileSeek(nx_file_t f, uint64_t position);
	NX_API ns_error_t NXFileTell(nx_file_t f, uint64_t *position);
	NX_API ns_error_t NXFileLockRegion(nx_file_t _f, uint64_t start_position, uint64_t end_position);
	NX_API ns_error_t NXFileUnlockRegion(nx_file_t _f);
	/* file_stats does _not_ take into account the current region */
	NX_API ns_error_t NXFileStat(nx_file_t f, nx_file_stat_t file_stats);
	/* returns the length of the file given the current region */
	NX_API ns_error_t NXFileLength(nx_file_t f, uint64_t *length);
	/* returns NErr_True, NErr_False, or possibly some error */
	NX_API ns_error_t NXFileEndOfFile(nx_file_t f);
	/* this exists as a one-off for nsmp4. hopefully we can get rid of it */
	NX_API ns_error_t NXFilePeekByte(nx_file_t f, uint8_t *byte);
	NX_API ns_error_t NXFileSync(nx_file_t f);
	NX_API ns_error_t NXFileTruncate(nx_file_t f);
#ifdef __cplusplus
}
#endif