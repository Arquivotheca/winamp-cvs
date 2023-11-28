#include "DirectWAVDecoder.h"
#include "nu/ByteReader.h"
#include "nswasabi/ReferenceCounted.h"


static const uint16_t nsiff_waveformat_pcm = 0x0001;
static const uint16_t nsiff_waveformat_extended = 0xFFFE;

// {00000001-0000-0010-8000-00aa00389b71}
static const GUID nsiff_waveformatex_pcm = 
{ 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };



ns_error_t PCMWAVDecoderService::WAVDecoderService_CreateDecoder(ifc_wavdecoder **out_decoder, nsiff_t iff_object, const void *fmt_chunk, size_t fmt_chunk_size, const void *fact_chunk, size_t fact_chunk_size)
{
	uint32_t total_length_samples=0;
	bytereader_s byte_reader;
	if (fmt_chunk_size < 16)
		return NErr_NeedMoreData;

	if (fact_chunk && fact_chunk_size >= 4)
	{
		bytereader_init(&byte_reader, fact_chunk, fact_chunk_size);
		total_length_samples = bytereader_read_u32_le(&byte_reader);
	}

	nsiff_waveformat waveformat;
	waveformat.sizeof_waveformat = fmt_chunk_size;

	
	bytereader_init(&byte_reader, fmt_chunk, fmt_chunk_size);
	waveformat.format_tag = bytereader_read_u16_le(&byte_reader);
	waveformat.channels = bytereader_read_u16_le(&byte_reader);
	waveformat.sample_rate = bytereader_read_u32_le(&byte_reader);
	waveformat.bytes_per_second = bytereader_read_u32_le(&byte_reader);
	waveformat.block_align = bytereader_read_u16_le(&byte_reader);
	waveformat.bps = bytereader_read_u16_le(&byte_reader);
	if (bytereader_size(&byte_reader) >= 2)
	{
		waveformat.extra_bytes = bytereader_read_u16_le(&byte_reader);
		if (waveformat.format_tag == nsiff_waveformat_extended)
		{
			if (bytereader_size(&byte_reader) < 22)
				return NErr_NeedMoreData;

				waveformat.extended.ex.valid_bits = bytereader_read_u16_le(&byte_reader);
				waveformat.extended.ex.channel_mask = bytereader_read_u32_le(&byte_reader);
				waveformat.extended.ex.guid = bytereader_read_uuid_le(&byte_reader);				
			}
			else if (waveformat.format_tag == 0x55)
			{
				if (bytereader_size(&byte_reader) < 12)
					return NErr_NeedMoreData;

				waveformat.extended.mp3.id = bytereader_read_u16_le(&byte_reader);
				waveformat.extended.mp3.flags = bytereader_read_u32_le(&byte_reader);
				waveformat.extended.mp3.block_size = bytereader_read_u16_le(&byte_reader);
				waveformat.extended.mp3.frames_per_block = bytereader_read_u16_le(&byte_reader);
				waveformat.extended.mp3.codec_delay = bytereader_read_u16_le(&byte_reader);
			}
	}

	if (waveformat.format_tag == nsiff_waveformat_pcm)
	{
		DirectWAVDecoder *decoder = new (std::nothrow) ReferenceCounted<DirectWAVDecoder>;
		if (!decoder)
			return NErr_OutOfMemory;

		ns_error_t ret = decoder->Initialize(&waveformat, total_length_samples);
		if (ret != NErr_Success)
		{
			delete decoder;
			return ret;
		}
		*out_decoder = decoder;
		return NErr_Success;
	}
	else if (waveformat.format_tag == nsiff_waveformat_extended)
	{
		if (waveformat.extended.ex.guid == nsiff_waveformatex_pcm)
		{
			DirectWAVDecoder *decoder = new (std::nothrow) ReferenceCounted<DirectWAVDecoder>;
		if (!decoder)
			return NErr_OutOfMemory;

		ns_error_t ret = decoder->Initialize(&waveformat, total_length_samples);
		if (ret != NErr_Success)
		{
			delete decoder;
			return ret;
		}
		*out_decoder = decoder;
		return NErr_Success;
		}
	}
	return NErr_False;
}

DirectWAVDecoder::DirectWAVDecoder()
{
	memset(&waveformat, 0, sizeof(waveformat));
	total_sample_length=0;
	buffer=0;
	buffer_size=0;
	sample_position=0;
}

	DirectWAVDecoder::~DirectWAVDecoder()
	{
		free(buffer);
	}

ns_error_t DirectWAVDecoder::Initialize(const nsiff_waveformat *waveformat, uint32_t total_sample_length)
{
	this->waveformat = *waveformat;
	total_sample_length = 0;

	buffer_size = waveformat->block_align * 4096;
	buffer = (uint8_t *)malloc(buffer_size);
	if (!buffer)
		return NErr_OutOfMemory;

	return NErr_Success;
}

ns_error_t DirectWAVDecoder::WAVDecoder_GetLengthSeconds(uint64_t data_length, double *length)
{
	if (total_sample_length)
		*length = (double)total_sample_length / (double)waveformat.sample_rate;
	else
		*length = (double)data_length / (double)waveformat.bytes_per_second;
	
	return NErr_Success;
}

ns_error_t DirectWAVDecoder::WAVDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters)
{
	parameters->audio.sample_rate = waveformat.sample_rate;
	if (waveformat.format_tag == nsiff_waveformat_pcm)
	{
		parameters->audio.format_type = nsaudio::format_type_pcm;
		if (waveformat.bps <= 8)
			parameters->audio.format_flags = nsaudio::FORMAT_FLAG_UNSIGNED|nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_LITTLE_ENDIAN;
		else
			parameters->audio.format_flags = nsaudio::FORMAT_FLAG_SIGNED|nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_LITTLE_ENDIAN;
		parameters->audio.bits_per_sample = waveformat.bps;
	}
	else if (waveformat.extended.ex.guid == nsiff_waveformatex_pcm)
	{
		parameters->audio.format_type = nsaudio::format_type_pcm;
		if (waveformat.bps <= 8)
			parameters->audio.format_flags = nsaudio::FORMAT_FLAG_UNSIGNED|nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_LITTLE_ENDIAN;
		else
			parameters->audio.format_flags = nsaudio::FORMAT_FLAG_SIGNED|nsaudio::FORMAT_FLAG_INTERLEAVED|nsaudio::FORMAT_FLAG_LITTLE_ENDIAN;
		parameters->audio.bits_per_sample = waveformat.extended.ex.valid_bits;
	}
	parameters->audio.bytes_per_sample = (waveformat.bps+7)/8;
	
	parameters->audio.number_of_channels = waveformat.channels;
	return NErr_Success;
}

ns_error_t DirectWAVDecoder::WAVDecoder_Decode(ifc_wavreader *reader, const void **output_buffer, size_t *output_buffer_bytes, double *start_position)
{
	*start_position = (double)sample_position / (double)waveformat.sample_rate;
	size_t bytes_read;
	ns_error_t ret = reader->ReadBytes(buffer, buffer_size, &bytes_read);
	if (ret != NErr_Success)
		return ret;

	*output_buffer = buffer;
	*output_buffer_bytes = bytes_read;

	sample_position += bytes_read / waveformat.block_align;
	return NErr_Success;
}

ns_error_t DirectWAVDecoder::WAVDecoder_SeekSeconds(double *seconds, uint64_t *new_data_offset)
{
	double seek_seconds = *seconds;
	uint64_t new_sample_position = (uint64_t)(seek_seconds * (double)waveformat.sample_rate);
	*new_data_offset = new_sample_position * waveformat.block_align;
	sample_position = new_sample_position;
	return NErr_Success;
}

ns_error_t DirectWAVDecoder::WAVDecoder_GetBitrate(double *bitrate)
{
	
	if (waveformat.extended.ex.guid == nsiff_waveformatex_pcm)
	{
		// TODO: validate this logic!
		*bitrate = waveformat.extended.ex.valid_bits * waveformat.channels * waveformat.sample_rate;
		return NErr_Success;
	}
	else
	{
		*bitrate = waveformat.bytes_per_second * 8;
		return NErr_Success;
	}

	
}