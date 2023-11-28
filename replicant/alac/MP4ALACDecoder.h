#pragma once
#include "mp4/ifc_mp4audiodecoder.h"
#include "nsalac/alac_decode.h"

class MP4ALACDecoder : public ifc_mp4audiodecoder
{
public:
	MP4ALACDecoder();
	~MP4ALACDecoder();
	int Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, alac_decoder_t decoder);
private:
	

	ifc_mp4file *mp4_file;
	ifc_mp4file::TrackID mp4_track;
	ifc_mp4file::SampleID next_sample;
	uint32_t max_sample_size;
	uint8_t *sample_buffer;
	alac_decoder_t decoder;
	size_t bytes_per_frame;

	/* ifc_mp4audiodecoder implementation */
	int WASABICALL MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters);
	int WASABICALL MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position);
	int WASABICALL MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number);
	int WASABICALL MP4AudioDecoder_SeekSeconds(double *seconds);
	int WASABICALL MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file);
};