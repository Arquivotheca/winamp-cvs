#include "OggPlaybackService.h"
#include "OggPlayback.h"
#include "nx/nxpath.h"
#include "nswasabi/ReferenceCounted.h"

ns_error_t OggPlaybackService::FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	nx_string_t ogg_extension = NXStringCreateFromUTF8("ogg");
	if (NXPathMatchExtension(filename, ogg_extension) == NErr_Success)
	{
		OggPlayback *ogg_playback = new (std::nothrow) ReferenceCounted<OggPlayback>();
		if (!ogg_playback)
			return NErr_OutOfMemory;

		int ret = ogg_playback->Initialize(filename, file, parent_metadata, parent);
		if (ret != NErr_Success)
		{
			delete ogg_playback;
			return ret;
		}

		*out_playback_object = ogg_playback;
		
		return NErr_Success;
	}
	return NErr_False;
}
