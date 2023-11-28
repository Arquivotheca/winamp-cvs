#pragma once
#include "mp4/ifc_mp4audiodecoder.h"
#include "nsmp3/mpgadecoder.h"

class MP4MP3Decoder : public ifc_mp4audiodecoder
{
public:
	static int CreateDecoder(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, ifc_mp4audiodecoder **out_decoder);
	
protected:
	MP4MP3Decoder();
	~MP4MP3Decoder();

private:
	int Initialize(ifc_mp4file *mp4_file, ifc_mp4file::TrackID mp4_track, uint8_t *sample_buffer, size_t max_sample_size);

	CMpgaDecoder *mp3;
	
	ifc_mp4file *mp4_file;
	ifc_mp4file::TrackID mp4_track;
	ifc_mp4file::SampleID next_sample;
	uint32_t max_sample_size;
	uint8_t *sample_buffer;
	unsigned long sample_rate;
	unsigned char channels;
	size_t pre_delay;
	float float_buffer[1152*2*2];

	/* ifc_mp4audiodecoder implementation */
	int WASABICALL MP4AudioDecoder_FillAudioParameters(ifc_audioout::Parameters *parameters);
	int WASABICALL MP4AudioDecoder_Decode(const void **output_buffer, size_t *output_buffer_bytes, double *start_position, double *end_position);
	int WASABICALL MP4AudioDecoder_Seek(ifc_mp4file::SampleID sample_number);
	int WASABICALL MP4AudioDecoder_SeekSeconds(double *seconds);
	int WASABICALL MP4AudioDecoder_ConnectFile(ifc_mp4file *new_file);
};