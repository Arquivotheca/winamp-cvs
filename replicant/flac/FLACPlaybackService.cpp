#include "FLACPlaybackService.h"
#include "FLACPlayback.h"
#include "nx/nxpath.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>
#include "main.h"

ns_error_t FLACPlaybackService::FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	if (NXPathMatchExtension(filename, flac_extension) == NErr_Success)
	{
		FLACPlayback *flac_playback = new (std::nothrow) ReferenceCounted<FLACPlayback>();
		if (!flac_playback)
			return NErr_OutOfMemory;

		int ret = flac_playback->Initialize(filename, file, parent_metadata, parent);
		if (ret != NErr_Success)
		{
			delete flac_playback;
			return ret;
		}

		*out_playback_object = flac_playback;
		
		return NErr_Success;
	}
	return NErr_False;
}
