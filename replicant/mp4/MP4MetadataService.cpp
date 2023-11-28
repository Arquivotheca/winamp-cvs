#include "main.h"
#include "MP4MetadataService.h"
#include "nx/nxstring.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include "MP4Metadata.h"
#include "MP4MetadataEditor.h"
#include <new>

nx_once_value_t MP4MetadataService::mime_once;

MP4MetadataService::MP4MetadataService()
{
	NXOnceInit(&mime_once);
}

int MP4MetadataService::MetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)
{
	return ::EnumerateExtensions(index, extension, EXTENSION_FOR_METADATA);
}


int MP4MetadataService::MetadataService_CreateMetadata(unsigned int pass, nx_uri_t filename, ifc_metadata **metadata)
{
#if 0
	if (IsMyExtension(filename, EXTENSION_FOR_METADATA))
	{
		MP4Metadata *mp4_metadata = new ReferenceCounted<MP4Metadata>;
		if (!mp4_metadata)
			return NErr_OutOfMemory;

		int ret = mp4_metadata->Initialize(filename);
		if (ret != NErr_Success)
		{
			mp4_metadata->Release();
			return ret;
		}

		*metadata  = mp4_metadata;
		return NErr_Success;
	}
#endif
	return NErr_False;
}

int MP4MetadataService::MetadataService_CreateMetadataEditor(unsigned int pass, nx_uri_t filename, ifc_metadata_editor **metadata)
{
	if (IsMyExtension(filename, EXTENSION_FOR_METADATA))
	{
		MP4MetadataEditor *mp4_metadata = new (std::nothrow) ReferenceCounted<MP4MetadataEditor>;
		if (!mp4_metadata)
			return NErr_OutOfMemory;

		int ret = mp4_metadata->Initialize(filename);
		if (ret != NErr_Success)
		{
			mp4_metadata->Release();
			return ret;
		}

		*metadata  = mp4_metadata;
		return NErr_Success;
	}
	return NErr_False;
}

int MP4FileMetadataService::FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)
{
	return ::EnumerateExtensions(index, extension, EXTENSION_FOR_METADATA);
}

int MP4FileMetadataService::FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	if (IsMyExtension(filename, EXTENSION_FOR_METADATA))
	{
		ReferenceCountedObject<MP4FileMetadata> mp4_metadata;

		if (!mp4_metadata)
			return NErr_OutOfMemory;

		int ret = mp4_metadata->Initialize(filename, file, parent_metadata);
		if (ret != NErr_Success)
			return ret;

		*file_metadata = mp4_metadata;
		mp4_metadata->Retain();
		return NErr_Success;
	}

	return NErr_False;
}

MP4FileMetadata::MP4FileMetadata()
{
}

MP4FileMetadata::~MP4FileMetadata()
{
}

ns_error_t MP4FileMetadata::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	ns_error_t ret = MP4MetadataFile::Initialize(filename, file);
	if (ret != NErr_Success)
		return ret;

	ret = SetParentMetadata(parent_metadata);
	if (ret != NErr_Success)
		return ret;


	return ret;
}

