#pragma once
#include "FLAC/all.h"
#include "FLACMetadataCallbacks.h"

MetadataReader::MetadataReader(nx_file_t file)
{
	this->file = NXFileRetain(file);
	NXFileStat(file, &file_stats);
}

MetadataReader::~MetadataReader()
{
	NXFileRelease(file);
}

nx_file_stat_t MetadataReader::GetFileStats()
{
	return &file_stats;
}

void MetadataReader::Close()
{
	NXFileRelease(file);
	file=0;
}

static size_t flac_nxfile_read(void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
		size_t bytes_read;
	NXFileRead(reader->file, ptr, size*nmemb, &bytes_read);
	return bytes_read / size;
}

static size_t flac_nxfile_write(const void *ptr, size_t size, size_t nmemb, FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	NXFileWrite(reader->file, ptr, size*nmemb);
	return nmemb;
}

static int flac_nxfile_seek(FLAC__IOHandle handle, FLAC__int64 offset, int whence)
{
	MetadataReader *reader = (MetadataReader *)handle;
	switch(whence)
	{
	case SEEK_SET:
		NXFileSeek(reader->file, offset);
		return 0;
	case SEEK_CUR:
		{
			uint64_t current;
			NXFileTell(reader->file, &current);
			current += offset;
			NXFileSeek(reader->file, current);
			return 0;
		}
	case SEEK_END:
		{
			uint64_t current;
			NXFileLength(reader->file, &current);
			current += offset;
			NXFileSeek(reader->file, current);
			return 0;
		}
	}
	
	return 1;
}

static FLAC__int64 flac_nxfile_tell(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	uint64_t position;
	NXFileTell(reader->file, &position);
	return position;
}

static int flac_nxfile_eof(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	if (NXFileEndOfFile(reader->file) == NErr_True)
		return 1;
	else
		return 0;
}

static int flac_nxfile_close(FLAC__IOHandle handle)
{
	MetadataReader *reader = (MetadataReader *)handle;
	int ret = 0;
	if (reader->file)
		NXFileRelease(reader->file);
	reader->file=0;
	return 0;
}

FLAC__IOCallbacks nxfile_io_callbacks =
{
  flac_nxfile_read,
  flac_nxfile_write,
  flac_nxfile_seek,
  flac_nxfile_tell,
  flac_nxfile_eof,
  flac_nxfile_close,
};

