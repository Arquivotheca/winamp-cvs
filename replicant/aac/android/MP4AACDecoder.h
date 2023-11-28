#pragma once
#include "mp4/ifc_mp4audiodecoder.h"
#include "IAACDecoder.h"

class MP4AACDecoder : public ifc_mp4audiodecoder
{
public:
	static int CreateDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder);
protected:
	MP4AACDecoder();
	~MP4AACDecoder();
private:
	int ConfigureDecoder(const uint8_t *asc, size_t asc_length);
	int Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track);
	IAACDecoder *decoder;
	ifc_mp4file *mp4_file;
	ifc_mp4file::TrackID mp4_track;
	ifc_mp4file::SampleID next_sample;
	uint32_t max_sample_size;
	uint8_t *sample_buffer;
	int16_t decoder_output[4096];

	/* ifc_mp4audiodecoder implementation */
	int WASABICALL MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters);
	int WASABICALL MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position);
	int WASABICALL MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number);
	int WASABICALL MP4AudioDecoder_SeekSeconds(double *seconds);
	int WASABICALL MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file);
};