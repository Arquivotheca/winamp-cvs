#pragma once
#include "FLAC/all.h"
#include "audio/ifc_audioout.h"
#include "file/ifc_fileplayback.h"
#include "FLACMetadata.h"
#include "nswasabi/MetadataChain.h"
#include "FLACFileCallbacks.h"

class FLACPlayback : public ifc_fileplayback
{
public:
	FLACPlayback();
	~FLACPlayback();
	int Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, ifc_fileplayback_parent *parent);
	nx_file_t file;
private:
	ifc_fileplayback_parent *parent;
	
	MetadataChain<FLACMetadata> *metadata;
	FLAC__StreamMetadata *stream_info;
	FLAC__StreamDecoder *decoder;

	const FLAC__int32 **output_pointers;
	unsigned int sample_rate;
	uint64_t flac_position;
	ns_error_t decode_ret;
	bool output_opened;
	uint64_t start_frame;
private:
	/* FLAC Callbacks */
	static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
	static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);

	FLAC__StreamDecoderWriteStatus Internal_OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);

	int Init(ifc_metadata *parent_metadata);

	void WASABICALL FilePlayback_Close();
	ns_error_t WASABICALL FilePlayback_Seekable();
	ns_error_t WASABICALL FilePlayback_GetMetadata(ifc_metadata **metadata);
	ns_error_t WASABICALL FilePlayback_GetLength(double *length, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_GetBitrate(double *bitrate, ns_error_t *exact);
	ns_error_t WASABICALL FilePlayback_Seek(const Agave_Seek *seek, ns_error_t *seek_error, double *new_position);
	ns_error_t WASABICALL FilePlayback_DecodeStep();
	ns_error_t WASABICALL FilePlayback_Interrupt(Agave_Seek *resume_information);
	ns_error_t WASABICALL FilePlayback_Resume(Agave_Seek *resume_information, nx_file_t file, ifc_metadata *parent_metadata);

	FLACClientData client_data;
};