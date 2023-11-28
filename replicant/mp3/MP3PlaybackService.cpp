#include "main.h"
#include "MP3PlaybackService.h"
#include "MP3Playback.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>

ns_error_t MP3PlaybackService::FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	if (IsMyExtension(filename))
	{
		MP3Playback *mp3_playback = new (std::nothrow) ReferenceCounted<MP3Playback>();
		if (!mp3_playback)
			return NErr_OutOfMemory;

		int ret = mp3_playback->Initialize(filename, file, parent_metadata, parent);
		if (ret != NErr_Success)
		{
			delete mp3_playback;
			return ret;
		}

		*out_playback_object = mp3_playback;
		
		return NErr_Success;
	}
	return NErr_False;
}