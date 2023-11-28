#include "api.h"
#include "FileRawReader.h"
#include "FileMetadata.h"
#include "nswasabi/ReferenceCounted.h"
#include <new>
#include "foundation/error.h"
#include "nx/nxpath.h"
#include "service/ifc_servicefactory.h"

static ns_error_t GetRawReader(ifc_raw_media_reader **out_reader, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(svc_filerawreader::GetServiceType(), n++))
	{
		svc_filerawreader *l = (svc_filerawreader*)sf->GetInterface();
		if (l)
		{
			ifc_raw_media_reader *reader=0;
			int ret = l->CreateRawMediaReader(&reader, filename, file, parent_metadata);
			l->Release();

			if (ret == NErr_Success && reader)
			{
				*out_reader = reader;
				return NErr_Success;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

int FileRawReaderService::RawMediaReaderService_CreateRawMediaReader(ifc_raw_media_reader **reader, nx_uri_t filename, unsigned int pass)
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

	ret = GetRawReader(reader, filename, f, file_metadata);
	NXFileRelease(f);
	return ret;
}
