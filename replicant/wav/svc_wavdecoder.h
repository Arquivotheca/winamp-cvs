#pragma once
#include "foundation/dispatch.h"
#include "foundation/error.h"
#include "nsiff/nsiff.h"

class ifc_wavdecoder;
class ifc_mp4videodecoder;

// {2467A320-849F-4689-BA92-4CAF7A54BD69}
static const GUID wav_decoder_service_type_guid = 
{ 0x2467a320, 0x849f, 0x4689, { 0xba, 0x92, 0x4c, 0xaf, 0x7a, 0x54, 0xbd, 0x69 } };

class svc_wavdecoder : public Wasabi2::Dispatchable
{
protected:
	svc_wavdecoder() : Dispatchable(DISPATCHABLE_VERSION) {}
	~svc_wavdecoder() {}
public:
		static GUID GetServiceType() { return wav_decoder_service_type_guid; }
		ns_error_t CreateDecoder(ifc_wavdecoder **out_decoder, nsiff_t iff_object, const void *fmt_chunk, size_t fmt_chunk_size, const void *fact_chunk, size_t fact_chunk_size) { return WAVDecoderService_CreateDecoder(out_decoder, iff_object, fmt_chunk, fmt_chunk_size, fact_chunk, fact_chunk_size); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual ns_error_t WASABICALL WAVDecoderService_CreateDecoder(ifc_wavdecoder **out_decoder, nsiff_t iff_object, const void *fmt_chunk, size_t fmt_chunk_size, const void *fact_chunk, size_t fact_chunk_size)=0;
};
