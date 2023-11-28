#include "MP4AACDecoder.h"
#include "foundation/error.h"
#include "nswasabi/ReferenceCounted.h"

int MP4AACDecoder::CreateDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder)
{
	uint8_t *buffer = NULL;
	uint32_t buffer_size = 0;
	uint8_t *sample_buffer = 0;
	CAccessUnitPtr access_unit = 0;
	CCompositionUnitPtr composition_unit = 0;
	mp4AudioDecoderHandle decoder = 0;
	CSAudioSpecificConfig asc;

	/* TODO: see if there's a dependent track, e.g. MPEG-4 SLS */

	/* read the decoder configuration data */
	if (mp4_file->Track_GetESConfiguration(mp4_track, &buffer, &buffer_size) != NErr_Success || !buffer)
		return NErr_WrongFormat;

	/* now try to parse it */
	memset(&asc, 0, sizeof(asc));
	if (mp4AudioDecoder_ascParse((const unsigned char *)buffer, buffer_size, &asc) != MP4AUDIODEC_OK)
	{
		mp4_file->Free(buffer);
		return NErr_WrongFormat;
	}

	/* get the maximum input size so we can make a buffer for it */
	uint32_t max_sample_size;
	if (mp4_file->Track_GetMaxSampleSize(mp4_track, &max_sample_size) != NErr_Success)
	{
		mp4_file->Free(buffer);
		return NErr_Error;
	}

	/* allocate memory for it */
	sample_buffer = (uint8_t *)malloc(max_sample_size);
	if (!sample_buffer)
	{
		mp4_file->Free(buffer);
		return NErr_OutOfMemory;
	}
		
	/* make an input object as used by the decoder (shame it can't use the same memory as above, oh well */
	access_unit = CAccessUnit_Create(0, 0);
	if (!access_unit)
	{
		mp4_file->Free(buffer);
		free(sample_buffer);
		return NErr_FailedCreate;
	}

	/* make the decoder itself */
	CSAudioSpecificConfig *asc_array = &asc;;
	decoder = mp4AudioDecoder_Create(&asc_array, 1);
	if (!decoder)
	{
		mp4_file->Free(buffer);
		free(sample_buffer);
		CAccessUnit_Destroy(&access_unit);
		return NErr_FailedCreate;
	}

	/* set configuration options */
	mp4AudioDecoder_SetParam(decoder, TDL_MODE, SWITCH_OFF); /* turn off limiter */
	mp4AudioDecoder_SetParam(decoder, CONCEALMENT_ENERGYINTERPOLATION, SWITCH_OFF); /* turn off concealment */

	/* create the output object as used by the decoder */
	composition_unit = CCompositionUnit_Create(max(asc.m_channels, 8), asc.m_samplesPerFrame * 2, asc.m_samplingFrequency, 6144, CUBUFFER_PCMTYPE_FLOAT);
	if (!composition_unit)
	{
		mp4_file->Free(buffer);
		free(sample_buffer);
		mp4AudioDecoder_Destroy(&decoder);
		CAccessUnit_Destroy(&access_unit);
		return NErr_FailedCreate;
	}
		
	/* create our C++ object */
	MP4AACDecoder *mp4_decoder = new ReferenceCounted<MP4AACDecoder>;
	if (!mp4_decoder)
	{
		mp4_file->Free(buffer);
		free(sample_buffer);
		mp4AudioDecoder_Destroy(&decoder);
		CAccessUnit_Destroy(&access_unit);
		CCompositionUnit_Destroy(&composition_unit);
		return NErr_OutOfMemory;
	}

	/* initialize */
	int ret = mp4_decoder->Initialize(mp4_file, mp4_track, decoder, access_unit, composition_unit, sample_buffer, max_sample_size);
	if (ret != NErr_Success)
	{
		mp4_decoder->Release();
		return ret;
	}

	*out_decoder = mp4_decoder;
	return NErr_Success;
}

MP4AACDecoder::MP4AACDecoder()
{	
	decoder=0;
	composition_unit=0; /* output */
	access_unit=0; /* input */

	mp4_file=0;
	mp4_track=mp4file_invalid_track_id;
	next_sample=1;
	sample_rate=0;
	channels=0;
	pre_delay=0;
	sample_buffer = 0;
	max_sample_size = 0;
	float_buffer = 0;
}

int MP4AACDecoder::Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, mp4AudioDecoderHandle decoder, CAccessUnitPtr access_unit, CCompositionUnitPtr composition_unit, uint8_t *sample_buffer, size_t max_sample_size)
{
	this->mp4_file = mp4_file;
	mp4_file->Retain();
	this->mp4_track = mp4_track;
	this->decoder = decoder;
	this->access_unit = access_unit;
	this->composition_unit = composition_unit;
	this->sample_buffer = sample_buffer;
	this->max_sample_size = max_sample_size;
	return NErr_Success;
}

MP4AACDecoder::~MP4AACDecoder()
{
	mp4AudioDecoder_Destroy(&decoder);
	CAccessUnit_Destroy(&access_unit);
	CCompositionUnit_Destroy(&composition_unit);
	free(sample_buffer);
	free(float_buffer);
	if (mp4_file)
		mp4_file->Release();
}

int MP4AACDecoder::MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)
{
	unsigned int sample_rate;
	unsigned int channels;
	MP4_RESULT result;
	result = CCompositionUnit_GetSamplingRate(composition_unit, &sample_rate);
	result = CCompositionUnit_GetChannels(composition_unit, &channels);

	if (sample_rate == 0 || channels == 0)
		return NErr_NeedMoreData;

	parameters->audio.sample_rate = sample_rate;
	parameters->audio.format_type = nsaudio::format_type_float;
	parameters->audio.format_flags = nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_NATIVE_ENDIAN|nsaudio::FORMAT_FLAG_SIGNED;
	parameters->audio.bytes_per_sample = 4;
	parameters->audio.bits_per_sample = 32;
	parameters->audio.number_of_channels = channels;

	return NErr_Success;
}

int MP4AACDecoder::MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position)
{
	uint32_t sample_size=max_sample_size;

	ifc_mp4file::Timestamp timestamp;
	ifc_mp4file::Duration duration;
	ifc_mp4file::Duration offset;
	if (mp4_file->Sample_Read(mp4_track, next_sample++, &sample_buffer, &sample_size, &timestamp, &duration, &offset) == NErr_Success)
	{
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp, start_position);
		mp4_file->Track_ConvertFromTimestamp(mp4_track, timestamp+duration, end_position);

		CAccessUnit_Reset(access_unit);
		CAccessUnit_Assign(access_unit, (const unsigned char *)sample_buffer, sample_size);
		CCompositionUnit_Reset(composition_unit);
		MP4_RESULT result = mp4AudioDecoder_DecodeFrame(decoder, &access_unit, composition_unit);

		if (result == MP4AUDIODEC_OK)
		{
			unsigned int channels;
			unsigned int samples_per_channel;
			if (CCompositionUnit_GetSamplesPerChannel(composition_unit, &samples_per_channel) != MP4AUDIODEC_OK
				||		CCompositionUnit_GetChannels(composition_unit, &channels) != MP4AUDIODEC_OK)
				return NErr_Error;
			const float *audio_output = 0;

			size_t num_samples = samples_per_channel * channels;
			size_t output_size = num_samples * sizeof(float);

			*output_buffer_bytes = output_size;
			CCompositionUnit_GetPcmPtr(composition_unit, &audio_output);
			if (!float_buffer)
			{
				float_buffer = (float *)malloc(output_size);
				if (!float_buffer)
					return NErr_OutOfMemory;
			}
			for (size_t i=0;i<num_samples;i++)
			{
				float_buffer[i] = audio_output[i] / 32768.0f;
			}
			*output_buffer = float_buffer;

			

			return NErr_Success;
		}
		else
		{
			return NErr_Error;
		}

	}
	else
		return NErr_EndOfFile;
}

int MP4AACDecoder::MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number)
{
	next_sample = sample_number;
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
	return NErr_Success;
}

int MP4AACDecoder::MP4AudioDecoder_SeekSeconds(double *seconds)
{
	ifc_mp4file::Duration duration;
	if (mp4_file->Track_ConvertToDuration(mp4_track, *seconds, &duration) != NErr_Success)
		return NErr_EndOfFile;

	ifc_mp4file::SampleID new_sample_id;
	if (mp4_file->Sample_GetFromDuration(mp4_track, duration, &new_sample_id) != NErr_Success)
		return NErr_Error;

	mp4_file->Track_ConvertFromTimestamp(mp4_track, duration, seconds);
	next_sample = new_sample_id;
	mp4AudioDecoder_Reset(decoder, MP4AUDIODECPARAM_DEFAULT, 0);
	return NErr_Success;	
}

int MP4AACDecoder::MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file)
{
	mp4_file->Release();
	mp4_file=new_file;
	mp4_file->Retain();
	return NErr_Success;
}