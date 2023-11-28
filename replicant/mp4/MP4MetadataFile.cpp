#include "MP4MetadataFile.h"

MP4MetadataFile::MP4MetadataFile()
{
	mp4_file=0;
}

MP4MetadataFile::~MP4MetadataFile()
{
	if (mp4_file)
		MP4Close(mp4_file);
	mp4_file=0;
}

int MP4MetadataFile::Initialize(nx_uri_t filename, nx_file_t file)
{
	mp4_file = MP4ReadFile(filename, file);
	if (!mp4_file)
	{
		return NErr_Error;
	}

	MP4CloseFile(mp4_file);
	return MP4MetadataBase::Initialize(filename, mp4_file);
}