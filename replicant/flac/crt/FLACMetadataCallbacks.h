#pragma once

#include "FLAC/all.h"
#include <stdio.h>
#include "nx/nxuri.h"
#include "nx/nxfile.h"
#include "foundation/error.h"

class MetadataReader
{
public:
	MetadataReader(nx_file_t file);
	~MetadataReader();
	void Close();
	nx_file_stat_t GetFileStats();
	
	nx_file_t file;
	nx_file_stat_s file_stats;
};

extern FLAC__IOCallbacks nxfile_io_callbacks;