#include "nxfile.h"
#include "foundation/error.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define _DARWIN_USE_64_BIT_INODE
#include <sys/stat.h>

FILE *NXFile_fopen(nx_uri_t filename, nx_file_FILE_flags_t flags)
{
	char fs_buffer[1024];
	CFURLGetFileSystemRepresentation((CFURLRef)filename, false,
									 (uint8_t *)fs_buffer,
									 1024
									 );
	
	if (0 != (flags & nx_file_FILE_read_text))
	{
		return fopen(fs_buffer, "r");
	}
	else if (0 != (flags & nx_file_FILE_write_text))
	{
		return fopen(fs_buffer, "w");
	}
	else if (0 != (flags & nx_file_FILE_update_text))
	{
		return fopen(fs_buffer, "r+");
	}
	else if (0 != (flags & nx_file_FILE_readwrite_text))
	{
		return fopen(fs_buffer, "w+");
	}
	
	return 0;
	
}

int NXFile_open(nx_uri_t filename, int flags)
{
	char fs_buffer[PATH_MAX];
	CFURLGetFileSystemRepresentation((CFURLRef)filename, false,
									 (uint8_t *)fs_buffer,
									 sizeof(fs_buffer)/sizeof(char));
	
	return open(fs_buffer, flags);
}

int NXFile_move(nx_uri_t destination, nx_uri_t source)
{
	return NErr_NotImplemented;
}

int NXFile_unlink(nx_uri_t filename)
{
	return NErr_NotImplemented;
}

int NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats)
{
	struct stat buffer; 
	char fs_buffer[PATH_MAX];
	Boolean result;
	
	result = CFURLGetFileSystemRepresentation((CFURLRef)filename, false,
											  (uint8_t *)fs_buffer,
											  sizeof(fs_buffer)/sizeof(char));
	
	if (false == result)
		return NErr_Error;
	
	if (stat(fs_buffer, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atimespec.tv_sec;
		file_stats->creation_time = buffer.st_birthtimespec.tv_sec;
		file_stats->modified_time = buffer.st_mtimespec.tv_sec;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}

int NXFile_statFILE(FILE *f, nx_file_stat_t file_stats)
{
	int fd = fileno(f);
	if (fd == -1)
		return NErr_Error;

	return NXFile_fstat(fd, file_stats);
}

int NXFile_fstat(int file_descriptor, nx_file_stat_t file_stats)
{
	struct stat buffer; 

	if (fstat(file_descriptor, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atimespec.tv_sec;
		file_stats->creation_time = buffer.st_birthtimespec.tv_sec;
		file_stats->modified_time = buffer.st_mtimespec.tv_sec;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}	