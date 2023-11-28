#include "nxfile.h"
#include "foundation/error.h"

/* Linux implementation */

	int NXFile_open(nx_uri_t filename, int flags)
	{
		return open(filename->string, flags|O_LARGEFILE);
	}

int NXFile_move(nx_uri_t destination, nx_uri_t source)
{
	if (rename(source->string, destination->string) == 0)
		return NErr_Success;
	else
		return NErr_Error;
}

int NXFile_unlink(nx_uri_t filename)
{
	if (unlink(filename->string) == 0)
		return NErr_Success;
	
	/* TODO: check errno */
	return NErr_Error;
}

int NXFile_stat(nx_uri_t filename, nx_file_stat_t file_stats)
{
	struct stat buffer;

	if (stat(filename->string, &buffer) == 0)
	{
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
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
		file_stats->access_time = buffer.st_atime;
		file_stats->creation_time = buffer.st_ctime;
		file_stats->modified_time = buffer.st_mtime;
		file_stats->file_size = buffer.st_size;
		return NErr_Success;
	}
	else
		return NErr_Error;
}	