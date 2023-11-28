#pragma once
#include "FLAC/all.h"
#include "http/svc_http_demuxer.h"
#include "http/ifc_http_demuxer.h"
#include "nx/nxstring.h"
#include "nswasabi/ServiceName.h"
#include "FLACMetadata.h"
#include "FLACFileCallbacks.h"

// {662DE658-9B07-4932-BE6C-2B5AEFF1488E}
static const GUID flac_demuxer_guid = 
{ 0x662de658, 0x9b07, 0x4932, { 0xbe, 0x6c, 0x2b, 0x5a, 0xef, 0xf1, 0x48, 0x8e } };


class FLACHTTPService : public svc_http_demuxer
{
public:	
	WASABI_SERVICE_NAME("FLAC HTTP Demuxer");
	static GUID GetServiceGUID() { return flac_demuxer_guid; }

	const char *WASABICALL HTTPDemuxerService_EnumerateAcceptedTypes(size_t i);
	const char *WASABICALL HTTPDemuxerService_GetUserAgent();
	void WASABICALL HTTPDemuxerService_CustomizeHTTP(jnl_http_t http);
	NError WASABICALL HTTPDemuxerService_CreateDemuxer(nx_uri_t uri, jnl_http_t http, ifc_http_demuxer **demuxer, int pass);
};

class FLACHTTP : public ifc_http_demuxer
{
public:
	FLACHTTP();
	~FLACHTTP();

	int Initialize(nx_uri_t uri, jnl_http_t http);
private:
	/* ifc_http_demuxer implementation */
	int WASABICALL HTTPDemuxer_Run(ifc_http *http_parent, ifc_player *player, ifc_playback_parameters *secondary_parameters);

	/* FLAC Callbacks */
	static FLAC__StreamDecoderWriteStatus OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data);
	static void OnMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
	FLAC__StreamDecoderWriteStatus Internal_OnAudio(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);

	/* member data */
	FLACMetadata *metadata;
	jnl_http_t http;
	nx_uri_t uri;
	nx_file_t file;
	FLAC__StreamMetadata *stream_info;
	FLAC__StreamDecoder *decoder;
	FLACClientData client_data;
	const FLAC__int32 **output_pointers;
	unsigned int sample_rate;
	ns_error_t decode_ret;
	uint64_t flac_position;
	ifc_audioout *out;
	ifc_player *player;
	ifc_http *http_parent;
	bool paused;

};
