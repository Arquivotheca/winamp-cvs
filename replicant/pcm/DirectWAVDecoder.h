#pragma once
#include "wav/svc_wavdecoder.h"
#include "wav/ifc_wavdecoder.h"
#include "nswasabi/ServiceName.h"
struct nsiff_waveformat_ex
{
	uint16_t valid_bits;
	uint32_t channel_mask;
	GUID guid;
};

struct nsiff_waveformat_mp3
{
	uint16_t id;
	uint32_t flags;
	uint16_t block_size;
	uint16_t frames_per_block;
	uint16_t codec_delay;
};

struct nsiff_waveformat
{
	size_t sizeof_waveformat;
	uint16_t format_tag;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t bytes_per_second;
	uint16_t block_align;
	uint16_t bps;
	uint16_t extra_bytes;
	union
	{
		nsiff_waveformat_ex ex;
		nsiff_waveformat_mp3 mp3;
	} extended;
};

// {17276C64-86AB-4DA6-86AD-DAF0E91CE7AF}
static const GUID pcm_wav_decoder_guid = 
{ 0x17276c64, 0x86ab, 0x4da6, { 0x86, 0xad, 0xda, 0xf0, 0xe9, 0x1c, 0xe7, 0xaf } };


class PCMWAVDecoderService : public svc_wavdecoder
{
public:
	WASABI_SERVICE_NAME("PCM WAV Decoder");
	static GUID GetServiceGUID() { return pcm_wav_decoder_guid; }
	ns_error_t WASABICALL WAVDecoderService_CreateDecoder(ifc_wavdecoder **out_decoder, nsiff_t iff_object, const void *fmt_chunk, size_t fmt_chunk_size, const void *fact_chunk, size_t fact_chunk_size);
private:
};

class DirectWAVDecoder : public ifc_wavdecoder
{
public:
	DirectWAVDecoder();
	~DirectWAVDecoder();
	ns_error_t Initialize(const nsiff_waveformat *waveformat, uint32_t total_sample_length);
private:
	ns_error_t WASABICALL WAVDecoder_GetLengthSeconds(uint64_t data_length, double *length);
	ns_error_t WASABICALL WAVDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters);
	ns_error_t WASABICALL WAVDecoder_Decode(ifc_wavreader *reader, const void **output_buffer, size_t *output_buffer_bytes, double *start_position);
	ns_error_t WASABICALL WAVDecoder_SeekSeconds(double *seconds, uint64_t *new_data_offset);
	ns_error_t WASABICALL WAVDecoder_GetBitrate(double *bitrate);

	nsiff_waveformat waveformat;
	uint32_t total_sample_length;
	uint8_t *buffer;
	size_t buffer_size;
	uint64_t sample_position;
};