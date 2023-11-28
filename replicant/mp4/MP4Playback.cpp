#include "api.h"
#include "MP4Playback.h"
#ifdef _WIN32
#include "nu/AutoChar.h"
#endif
#include "service/ifc_servicefactory.h"
#include "mp4/svc_mp4decoder.h"
#include "replaygain/ifc_replaygain_settings.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "nswasabi/ReferenceCounted.h"
#include "MP4FileObject.h"
#include <new>

MP4Playback::MP4Playback()
{
	audio_decoder=0;
	mp4_file=MP4_INVALID_FILE_HANDLE;
	samples_per_second=0;
	output_opened=false;
	mp4_file_object=0;
	parent=0;
	filename=0;
}

MP4Playback::~MP4Playback()
{
	if (mp4_file)
		MP4CloseFile(mp4_file); /* only close the file, mp4_file_object actually owns the handle */

	if (mp4_file_object)
		mp4_file_object->ifc_mp4file::Release();

	if (audio_decoder)
		audio_decoder->Release();

	NXURIRelease(filename);
}

int MP4Playback::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	this->parent = parent;
	this->filename = NXURIRetain(filename);	

	ns_error_t ret = Init(file, parent_metadata);
	if (ret != NErr_Success)
		return ret;
	return Configure();	
}

int MP4Playback::Init(nx_file_t file, ifc_metadata *parent_metadata)
{
	mp4_file = MP4ReadFile(filename, file, 0);

	if (!mp4_file)
		return NErr_FileNotFound;

	mp4_file_object = new (std::nothrow) ReferenceCounted<MetadataChain<MP4FileObject> >;
	if (!mp4_file_object)
	{
		/* manually close here.  mp4_file_object "owns" mp4_file but we weren't able to make it */
		MP4Close(mp4_file);
		mp4_file=0; 
		return NErr_OutOfMemory;
	}

	mp4_file_object->Initialize(filename, mp4_file);
	mp4_file_object->SetParentMetadata(parent_metadata);

	return NErr_Success;
}

static ifc_mp4audiodecoder *GetAudioDecoder(MP4FileHandle mp4_file, MP4TrackId mp4_track, ifc_mp4file *mp4_file_object)
{

	GUID mp4_decoder_guid = svc_mp4decoder::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(mp4_decoder_guid, n++))
	{
		svc_mp4decoder *l = (svc_mp4decoder*)sf->GetInterface();
		if (l)
		{
			ifc_mp4audiodecoder *decoder=0;
			int ret = l->CreateAudioDecoder(mp4_file_object, mp4_track, &decoder);
			l->Release();

			if (ret == NErr_Success && decoder)
				return decoder;
		}
	}
	return 0;
}

int MP4Playback::Configure()
{
	/* find appropriate audio decoder plugin */
	uint32_t num_audio_tracks = MP4GetNumberOfTracks(mp4_file, MP4_AUDIO_TRACK_TYPE);
	for (uint32_t track_num=0;track_num < num_audio_tracks && !audio_decoder;track_num++)
	{
		MP4TrackId audio_track = MP4FindTrackId(mp4_file, track_num, MP4_AUDIO_TRACK_TYPE);
		/* TODO: verify that this isn't a 'dependent' track, e.g. SLS portion of HD-AAC, stored in tref.dpnd */
		if (audio_track != MP4_INVALID_TRACK_ID)
			audio_decoder = GetAudioDecoder(mp4_file, audio_track, mp4_file_object);
	}

	if (!audio_decoder)
	{
		return NErr_NoMatchingImplementation;
	}
	return NErr_Success;
}

void MP4Playback::FilePlayback_Close()
{
	if (mp4_file)
		MP4CloseFile(mp4_file); /* only close the file, mp4_file_object actually owns the handle */
	mp4_file=0;

	if (mp4_file_object)
		mp4_file_object->ifc_mp4file::Release();
	mp4_file_object=0;

	if (audio_decoder)
		audio_decoder->Release();
	audio_decoder=0;
}

ns_error_t MP4Playback::FilePlayback_Seekable()
{
	return NErr_True;
}

ns_error_t MP4Playback::FilePlayback_GetMetadata(ifc_metadata **metadata)
{
	if (mp4_file_object)
	{

		*metadata = mp4_file_object;
		(*metadata)->Retain();
		return NErr_Success;
	}
	return NErr_Empty;
}

ns_error_t MP4Playback::FilePlayback_GetLength(double *length, ns_error_t *exact)
{
	MP4Duration file_duration = MP4GetDuration(mp4_file);
	*length = MP4ConvertFromMovieDuration(mp4_file, file_duration, MP4_USECS_TIME_SCALE) / (double)MP4_USECS_TIME_SCALE;
	*exact = NErr_True;
	return NErr_Success;
}

ns_error_t MP4Playback::FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)
{

	return NErr_NotImplemented; // TODO
}

ns_error_t MP4Playback::FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)
{
	switch(seek->position_type)
	{
	case AGAVE_PLAYPOSITION_SECONDS:
		double position = seek->position.seconds;
		audio_decoder->SeekSeconds(&position);
		*new_position = position;
		*seek_error = NErr_Success;
		return NErr_Success;
	}
	return NErr_NotImplemented;
}

ns_error_t MP4Playback::FilePlayback_DecodeStep()
{
	/* decode next audio sample */
	const uint8_t *decoded_data=0;
	size_t decoded_bytes=0;
	double decoded_position=0, end_position=0;

	ns_error_t ret = audio_decoder->Decode((const void **)&decoded_data, &decoded_bytes, &decoded_position, &end_position);
	if (ret == NErr_Success)
	{
		if (!output_opened)
		{
			memset(&audio_parameters, 0, sizeof(ifc_audioout::Parameters));
			audio_parameters.sizeof_parameters = sizeof(ifc_audioout::Parameters);
			ret = audio_decoder->FillAudioParameters(&audio_parameters);
			if (ret != NErr_Success)
				return ret;

			samples_per_second = audio_parameters.audio.sample_rate * audio_parameters.audio.number_of_channels;
			ret = parent->OpenOutput(&audio_parameters);
			if (ret != NErr_Success)
				return ret;

			output_opened=true;
		}

		if (decoded_bytes)
		{
			size_t consumed_frames; // TODO: if we get an NErr_Interrupted, we should save how many bytes we are into the MP4Sample, save the decoded data, and continue from there
			if (audio_parameters.audio.format_flags & nsaudio::FORMAT_FLAG_NONINTERLEAVED)
			{
				ret = parent->OutputNonInterleaved(decoded_data, decoded_bytes, &consumed_frames, decoded_position);
			}
			else
			{
				ret = parent->Output(decoded_data, decoded_bytes, &consumed_frames, decoded_position);
			}
			if (ret != NErr_Success)
				return ret;
		}
		return NErr_Success;
	}
	else if (ret == NErr_EndOfFile)
	{
		return NErr_EndOfFile;
	}
	else
	{
		return ret;			
	}
}

ns_error_t MP4Playback::FilePlayback_Interrupt(Agave_Seek *resume_information)
{
	if (mp4_file)
		MP4CloseFile(mp4_file);
	mp4_file=0;

	if (mp4_file_object)
		mp4_file_object->ifc_mp4file::Release();
	mp4_file_object=0;

	return NErr_Success;			
}

ns_error_t MP4Playback::FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)
{
	ns_error_t ret = Init(file, parent_metadata);
	if (ret != NErr_Success)
		return ret;

	if (audio_decoder)
		audio_decoder->ConnectFile(mp4_file_object);


	return NErr_Success;
}
