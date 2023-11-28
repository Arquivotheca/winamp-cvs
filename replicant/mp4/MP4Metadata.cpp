#include "MP4Metadata.h"

MP4Metadata::MP4Metadata()
{
	mp4_file=0;
}

MP4Metadata::~MP4Metadata()
{
	if (mp4_file)
		MP4Close(mp4_file);
	mp4_file=0;
}

int MP4Metadata::Initialize(nx_uri_t filename)
{
	mp4_file = MP4Read(filename);
	if (!mp4_file)
	{
		return NErr_Error;
	}

	MP4CloseFile(mp4_file);
	return MP4MetadataBase::Initialize(filename, mp4_file);
}
