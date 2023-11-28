#include "api.h"
#include "FileMetadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include "svc_filemetadata.h"
#include "service/ifc_servicefactory.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

int FileMetadataService::MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)
{
	GUID filemetadata_guid = svc_filemetadata::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(filemetadata_guid, n++))
	{
		svc_filemetadata *l = (svc_filemetadata*)sf->GetInterface();
		if (l)
		{
			unsigned int i=0;
			for (;;)
			{
				nx_string_t local_extension;
				int ret = l->EnumerateExtensions(i++, &local_extension);
				if (ret == NErr_Success)
				{
					if (index == 0)
					{
						*extension = local_extension;
						l->Release();
						return NErr_Success;
					}
					else
					{
						index--;
						NXStringRelease(local_extension);
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	
	return NErr_EndOfEnumeration;
}

static ns_error_t GetFileMetadata(ifc_metadata **out_filemetadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	GUID filemetadata_guid = svc_filemetadata::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(filemetadata_guid, n++))
	{
		svc_filemetadata *l = (svc_filemetadata*)sf->GetInterface();
		if (l)
		{
			ifc_metadata *metadata=0;
			int ret = l->CreateFileMetadata(&metadata, filename, file, parent_metadata);
			l->Release();

			if (ret == NErr_Success && metadata)
			{
				*out_filemetadata = metadata;
				return NErr_Success;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

static ns_error_t GetFileMetadataEditor(ifc_filemetadata_editor **out_filemetadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata)
{
	GUID filemetadata_guid = svc_filemetadata::GetServiceType();
	ifc_serviceFactory *sf;
	size_t n = 0;
	while (sf = WASABI2_API_SVC->EnumService(filemetadata_guid, n++))
	{
		svc_filemetadata *l = (svc_filemetadata*)sf->GetInterface();
		if (l)
		{
			ifc_filemetadata_editor *metadata=0;
			int ret = l->CreateFileMetadataEditor(&metadata, filename, file, parent_metadata);
			l->Release();

			if (ret == NErr_Success && metadata)
			{
				*out_filemetadata = metadata;
				return NErr_Success;
			}
		}
	}
	return NErr_NoMatchingImplementation;
}

int FileMetadataService::MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **out_metadata)
{
	if (NXPathIsURL(filename) == NErr_True)
		return NErr_False;

	if (pass == 0)
		return NErr_TryAgain;

	nx_file_t f;
	ns_error_t ret = NXFileOpenFile(&f, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	FileMetadataRead *file_metadata = new (std::nothrow) ReferenceCounted<FileMetadataRead>();
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
		delete file_metadata;
		return ret;
	}

	ifc_metadata *sub_metadata;
	
	ret = GetFileMetadata(&sub_metadata, filename, f, file_metadata);
	NXFileRelease(f);
	if (ret == NErr_Success)
	{
		file_metadata->Release();
		*out_metadata = sub_metadata;
		return NErr_Success;
	}
	
	if (file_metadata->HasMetadata())
	{
		// fallback: just pass back our own internal metadata
		*out_metadata = file_metadata;	
		return NErr_Success;
	}
	else
	{
		file_metadata->Release();
		return NErr_False;
	}
}

int FileMetadataService::MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **out_metadata)
{
	if (NXPathIsURL(filename) == NErr_True)
		return NErr_False;

	if (pass == 0)
		return NErr_TryAgain;

	nx_file_t f;
	ns_error_t ret = NXFileOpenFile(&f, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	ReferenceCountedObject<FileMetadataWrite> file_metadata;
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

	ReferenceCountedPointer<ifc_filemetadata_editor> sub_metadata;
	ret = GetFileMetadataEditor(&sub_metadata, filename, f, file_metadata);
	NXFileRelease(f);
	if (ret == NErr_Success)
	{
		ret = file_metadata->Initialize(sub_metadata);
		if (ret != NErr_Success)
			return ret;

		return sub_metadata->GetMetdataObject(out_metadata);
	}
	
	if (file_metadata->HasMetadata())
	{
		// fallback: just pass back our own internal metadata
		*out_metadata = file_metadata;	
		return NErr_Success;
	}
	else
	{
		file_metadata->Release();
		return NErr_False;
	}	
}