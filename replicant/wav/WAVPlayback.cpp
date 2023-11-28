#include "api.h"
#include "main.h"
#include "WAVPlayback.h"
#include "service/ifc_servicefactory.h"
#include "replaygain/ifc_replaygain_settings.h"
#include "nswasabi/ReferenceCounted.h"
#include "svc_wavdecoder.h"
#include <new>


int WAVPlaybackService::FilePlaybackService_CreatePlayback(ifc_fileplayback **out_playback_object, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	if (IsMyExtension(filename, EXTENSION_FOR_PLAYBACK))
	{
		WAVPlayback *wav_playback = new (std::nothrow) WAVPlayback;
		if (!wav_playback)
		{
			return NErr_OutOfMemory;
		}

				int ret = wav_playback->Initialize(filename, file, parent_metadata, parent);
		if (ret != NErr_Success)
		{
			delete wav_playback;
			return ret;
		}

		*out_playback_object = wav_playback;
		return NErr_Success;
	}
	return NErr_False;
}


WAVPlayback::WAVPlayback()
{
	iff_object=0;
	output_opened=0;
	decoder=0;	
	metadata=0;
	parent=0;
}

WAVPlayback::~WAVPlayback()
{
	if (iff_object)
		nsiff_destroy(iff_object);

	if (decoder)
		decoder->Release();

	if (metadata)
		metadata->Release();
}

static ns_error_t GetDecoder(ifc_wavdecoder **out_decoder, nsiff_t iff_object, const void *fmt_chunk, size_t fmt_chunk_size, const void *fact_chunk, size_t fact_chunk_size)
{
	GUID wav_decoder_guid = svc_wavdecoder::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(wav_decoder_guid, n++))
	{
		svc_wavdecoder *l = (svc_wavdecoder*)sf->GetInterface();
		if (l)
		{
			ns_error_t ret = l->CreateDecoder(out_decoder, iff_object, fmt_chunk, fmt_chunk_size, fact_chunk, fact_chunk_size);
			l->Release();

			if (ret == NErr_Success)
				return NErr_Success;
			if (ret != NErr_False)
				return ret;
		}
	}
	return NErr_NoMatchingImplementation;
}

int WAVPlayback::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent)
{
	this->parent = parent;

	int ret = Init(file, parent_metadata);
	if (ret != NErr_Success)
		return ret;


	/* find decoder */
	ret = GetDecoder(&decoder, iff_object, fmt_chunk, fmt_chunk_size, fact_chunk, fact_chunk_size);
	if (ret != NErr_Success)
	{
		nsiff_destroy(iff_object);
		iff_object=0;
		return ret;
	}

	return NErr_Success;
}

ns_error_t WAVPlayback::Init(nx_file_t file, ifc_metadata *parent_metadata)
{
	metadata = parent_metadata;
	if (metadata)
		metadata->Retain();

	parse_error = NErr_Success;
	ns_error_t ret = nsiff_create_parser_from_file(&iff_object, file, &iff_callbacks, (WAVParser *)this);
	if (ret != NErr_Success)
	{
		return ret;
	}

	ret = nsiff_parse(iff_object);
	if (ret != NErr_Success)
	{
		if (parse_error != NErr_Success)
			return parse_error;
		else
			return ret;
	}

	if (data_position == 0)
	{
		nsiff_destroy(iff_object);
		iff_object=0;
		return NErr_Empty;
	}

	nsiff_seek(iff_object, data_position);
	return NErr_Success;
}

void WAVPlayback::FilePlayback_Close()
{
	if (iff_object)
		nsiff_destroy(iff_object);
	iff_object=0;

	if (decoder)
		decoder->Release();
	decoder=0;
}

ns_error_t WAVPlayback::FilePlayback_Seekable()
{
	// TODO:
	return NErr_True;
}

ns_error_t WAVPlayback::FilePlayback_GetMetadata(ifc_metadata **out_metadata)
{
	if (metadata)
	{
		*out_metadata = metadata;
		metadata->Retain();
	return NErr_Success;
	}
	else
		return NErr_Empty;
}

ns_error_t WAVPlayback::FilePlayback_GetLength(double *length, ns_error_t *exact)
{
	if (decoder->GetLengthSeconds(data_size, length) == NErr_Success)
	{
		*exact = NErr_True;
		return NErr_Success;
	}

	return NErr_Unknown;
}

ns_error_t WAVPlayback::FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact)
{
	int ret = decoder->GetBitrate(bitrate);
	if (ret == NErr_Success)
		*exact = NErr_True;

	return ret;
}

ns_error_t WAVPlayback::FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position)
{
	double seconds = seek->position.seconds;
	uint64_t new_data_offset;
	ns_error_t ret = decoder->SeekSeconds(&seconds, &new_data_offset);
	if (ret != NErr_Success)
		*seek_error = ret;

	*new_position = seconds;
	nsiff_seek(iff_object, data_position+new_data_offset);
	return NErr_Success;
}

ns_error_t WAVPlayback::FilePlayback_DecodeStep()
{
	const void *buffer;
	size_t buffer_size;
	double start_position;
	// we need to decode one frame first, to let compressed codecs better know their length
	ns_error_t ret = decoder->Decode(this, &buffer, &buffer_size, &start_position);
	if (ret != NErr_Success)
		return ret;

	if (!output_opened)
	{
		ifc_audioout::Parameters parameters={sizeof(ifc_audioout::Parameters), };
		
		ret = decoder->FillAudioParameters(&parameters);
		if (ret != NErr_Success)
			return ret;

		ret = parent->OpenOutput(&parameters);
		if (ret != NErr_Success)
			return ret;

		output_opened = true;
	}

	size_t frames_consumed; // TODO: if we wanted to be exact on interrupt/resume, we'd remember our offset for next time on NErr_Interrupted
	return parent->Output(buffer, buffer_size, &frames_consumed, start_position);
}

ns_error_t WAVPlayback::FilePlayback_Interrupt(Agave_Seek *resume_information)
{
	uint64_t absolute_position;
	nsiff_tell(iff_object, &absolute_position);
	resume_information->position.bytes = absolute_position - data_position;

	if (iff_object)
		nsiff_destroy(iff_object);
	iff_object=0;

	if (metadata)
		metadata->Release();
	metadata=0;

	return NErr_Success;
}

ns_error_t WAVPlayback::FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata)
{
	int ret = Init(file, parent_metadata);
	if (ret != NErr_Success)
		return ret;

	return nsiff_seek(iff_object, data_position+resume_information->position.bytes);	
}

ns_error_t WAVPlayback::WAVReader_ReadBytes(void *data, size_t bytes_requested, size_t *bytes_read)
{
	// TODO: bind to region
	return nsiff_read(iff_object, data, bytes_requested, bytes_read);
}

