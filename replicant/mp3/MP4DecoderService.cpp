#include "MP4DecoderService.h"
#include "foundation/error.h"
#include "MP4MP3Decoder.h"

int MP4DecoderService::MP4DecoderService_CreateAudioDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **decoder)
{
	const char *media_data_name;

	if (mp4_file->Track_GetMediaDataName(mp4_track, &media_data_name) != NErr_Success || memcmp(media_data_name, "mp4a", 4))
	{
		return NErr_False;
	}

	uint8_t audio_type;
	if (mp4_file->Track_GetESDSObjectTypeID(mp4_track, &audio_type) != NErr_Success)
		return NErr_False;
	switch(audio_type)
	{
	case ifc_mp4file::esds_object_type_mpeg1_audio:
	case ifc_mp4file::esds_object_type_mpeg2_audio:
		break;
	case ifc_mp4file::esds_object_type_mpeg4_audio:
		{
			uint8_t mpeg4_type;
			if (mp4_file->Track_GetAudioMPEG4Type(mp4_track, &mpeg4_type) != NErr_Success)
				return NErr_False;
			switch (mpeg4_type)
			{
			case ifc_mp4file::mpeg4_audio_type_layer1:
			case ifc_mp4file::mpeg4_audio_type_layer2:
			case ifc_mp4file::mpeg4_audio_type_layer3:
				break;
			default:
				return NErr_False;
			}
		}
	default:
		return NErr_False;
	}



	int ret = MP4MP3Decoder::CreateDecoder(mp4_file, mp4_track, decoder);
	if (ret)
		return ret;

	return NErr_Success;	

}