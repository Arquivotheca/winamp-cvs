#include "MP4DecoderService.h"
#include "foundation/error.h"
#include "nsalac/alac_decode.h"
#include "nswasabi/ReferenceCounted.h"
#include "MP4ALACDecoder.h"

int MP4DecoderService::MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder)
{
	const char *media_data_name;

	if (mp4_file->Track_GetMediaDataName(mp4_track, &media_data_name) != NErr_Success || memcmp(media_data_name, "alac", 4))
		return NErr_False;

	uint8_t *buffer;
	uint32_t buffer_size;
	int ret = mp4_file->Track_GetBytesProperty(mp4_track, "mdia.minf.stbl.stsd.alac.alac.decoderConfig", &buffer, &buffer_size);
	if (ret != NErr_Success)
		return NErr_False;


	alac_decoder_t alac=0;
	ret = alac_create(&alac);
	if (ret != NErr_Success)
	{
		mp4_file->Free(buffer);
		return ret;
	}

	ret = alac_configure(alac, buffer, buffer_size);
	mp4_file->Free(buffer);
	if (ret != NErr_Success)
	{
		alac_destroy(alac);
		return ret;
	}

	MP4ALACDecoder *decoder = new (std::nothrow) ReferenceCounted<MP4ALACDecoder>;
	if (!decoder)
	{
		alac_destroy(alac);
		return NErr_OutOfMemory;
	}

	ret = decoder->Initialize(mp4_file, mp4_track, alac);
	if (ret != NErr_Success)
	{
		delete decoder;
		return ret;
	}
	
	*out_decoder = decoder;
	return NErr_Success;	
	}