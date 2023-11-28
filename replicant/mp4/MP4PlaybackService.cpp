#include "main.h"
#include "MP4PlaybackService.h"
#include "MP4Playback.h"
#include "nx/nxpath.h"
#include "nswasabi/ReferenceCounted.h"

ns_error_t MP4PlaybackService::FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	if (IsMyExtension(filename, EXTENSION_FOR_PLAYBACK))
	{
		MP4Playback *mp4_playback = new ReferenceCounted<MP4Playback>;
		if (!mp4_playback)
			return NErr_OutOfMemory;

		int ret = mp4_playback->Initialize(filename, file, parent_metadata, parent);
		if (ret != NErr_Success)
		{
			mp4_playback->Release();
			return ret;
		}

		*out_playback_object = mp4_playback;
		
		return NErr_Success;
	}
	return NErr_False;
}
