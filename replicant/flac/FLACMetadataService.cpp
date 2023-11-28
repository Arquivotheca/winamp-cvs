#include "api.h"
#include "main.h"
#include "FLACMetadataService.h"
#include "nx/nxstring.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include <new>
//#ifdef _WIN32
//#include "windows/FLACMetadataCallbacks.h"
//#endif




int FLACMetadataService::FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)
{
	if (index == 0)
	{
		*extension = NXStringRetain(flac_extension);
		return NErr_Success;
	}

	return NErr_False;
}

int FLACMetadataService::FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	if (NXPathMatchExtension(filename, flac_extension) == NErr_Success)
	{
		ReferenceCountedObject<FLACFileMetadata> flac_metadata;

		if (!flac_metadata)
			return NErr_OutOfMemory;

		int ret = flac_metadata->Initialize(filename, file, parent_metadata);
		if (ret != NErr_Success)
			return ret;

		*file_metadata = flac_metadata;
		flac_metadata->Retain();
		return NErr_Success;
	}

	return NErr_False;
}

int FLACMetadataService::FileMetadataService_CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata)
{
	if (NXPathMatchExtension(filename, flac_extension) == NErr_Success)
	{
		ReferenceCountedObject<FLACFileMetadataEditor> flac_metadata;

		if (!flac_metadata)
			return NErr_OutOfMemory;

		int ret = flac_metadata->Initialize(filename, file, parent_metadata);
		if (ret != NErr_Success)
			return ret;

		*file_metadata = flac_metadata;
		flac_metadata->Retain();
		return NErr_Success;
	}

	return NErr_False;
}


ns_error_t FLACFileMetadata::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{

	ns_error_t ret = MetadataChain<FLACMetadataEditor>::Initialize(filename, file, false);
	if (ret != NErr_Success)
	{
		return ret;
	}

	return MetadataChain<FLACMetadataEditor>::SetParentMetadata(parent_metadata);
}

/* ------------ */
ns_error_t FLACFileMetadataEditor::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata)
{
	ns_error_t ret = MetadataEditorChain<FLACMetadataEditor>::Initialize(filename, file, false);
	if (ret != NErr_Success)
	{
		return ret;
	}

	return MetadataEditorChain<FLACMetadataEditor>::SetParentMetadata(parent_metadata);
}

ns_error_t FLACFileMetadataEditor::FileMetadata_GetMetdataObject(ifc_metadata_editor **out_metadata)
{
	*out_metadata = (MetadataEditorChain<FLACMetadataEditor> *)this;
	Retain();
	return NErr_Success;
}

ns_error_t FLACFileMetadataEditor::FileMetadata_Save(nx_file_t file)
{
	return Internal_Save(file);
}

ns_error_t FLACFileMetadataEditor::FileMetadata_RequireTempFile()
{
	if (NeedsTempFile())
		return NErr_True;
	return NErr_False;
}

ns_error_t FLACFileMetadataEditor::FileMetadata_SaveAs(nx_file_t destination, nx_file_t source)
{
	return Internal_SaveAs(destination, source);
}

ns_error_t FLACFileMetadataEditor::FileMetadata_WantID3v2(int *position)
{
	return NErr_False;
}

ns_error_t FLACFileMetadataEditor::FileMetadata_WantID3v1()
{
	return NErr_False;
}

ns_error_t FLACFileMetadataEditor::FileMetadata_WantAPEv2(int *position)
{
	return NErr_False;
}

ns_error_t FLACFileMetadataEditor::FileMetadata_WantLyrics3()
{
	return NErr_False;
}