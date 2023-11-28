#include "api.h"
#include "FileDecoder.h"
#include "nswasabi/ReferenceCounted.h"
#include "FileMetadata.h"
#include "nx/nxpath.h"
#include "service/ifc_servicefactory.h"
#include "svc_filedecode.h"

static ns_error_t GetFileDecoder_Callback(ifc_audio_decoder_callback **out_decoder, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata, nsaudio::Parameters *parameters, int flags)
{
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(svc_filedecode::GetServiceType(), n++))
	{
		svc_filedecode *l = (svc_filedecode*)sf->GetInterface();
		if (l)
		{
			ifc_audio_decoder_callback *decoder=0;
			int ret = l->CreateAudioDecoder_Callback(&decoder, filename, file, parent_metadata, parameters, flags);
			l->Release();

			if (ret == NErr_Success && decoder)
			{
				*out_decoder = decoder;
				return NErr_Success;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

int FileDecoderService::DecodeService_CreateAudioDecoder_Callback(unsigned int pass, ifc_audio_decoder_callback **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)
{
	if (NXPathIsURL(filename) == NErr_True)
		return NErr_False;

	if (pass == 0)
		return NErr_TryAgain;

	nx_file_t f;
	ns_error_t ret = NXFileOpenFile(&f, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	ReferenceCountedObject<FileMetadataRead> file_metadata;
	if (!file_metadata)
	{
		NXFileRelease(f);
		return NErr_OutOfMemory;
	}

	nx_file_stat_s file_stats;
	NXFileStat(f, &file_stats);
	file_metadata->SetFileInformation(filename, &file_stats);
	ret = file_metadata->FindMetadata(f);
	
	if (ret != NErr_Success)
	{
		NXFileRelease(f);
		return ret;
	}

	ret = GetFileDecoder_Callback(decoder, filename, f, file_metadata, parameters, flags);
	NXFileRelease(f);
	return ret;
}

int FileDecoderService::DecodeService_CreateAudioDecoder_Pull(unsigned int pass, ifc_audio_decoder_pull **decoder, nx_uri_t filename, nsaudio::Parameters *parameters, int flags)
{
	return NErr_False;
}
