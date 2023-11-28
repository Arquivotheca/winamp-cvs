#include "main.h"
#include "MP3MetadataService.h"
#include "nx/nxstring.h"
#include "nswasabi/ReferenceCounted.h"
#include "nx/nxpath.h"
#include "giofile_crt.h"
#include <new>

int MP3MetadataService::FileMetadataService_EnumerateExtensions(unsigned int index, nx_string_t *extension)
{
	return ::EnumerateExtensions(index, extension);
}

int MP3MetadataService::FileMetadataService_CreateFileMetadata(ifc_metadata **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	if (IsMyExtension(filename))
	{
		ReferenceCountedObject<MP3FileMetadata> mp3_metadata;

		if (!mp3_metadata)
			return NErr_OutOfMemory;

		int ret = mp3_metadata->Initialize(filename, file, parent_metadata);
		if (ret != NErr_Success)
			return ret;

		*file_metadata = mp3_metadata;
		mp3_metadata->Retain();
		return NErr_Success;
	}

	return NErr_False;
}

int MP3MetadataService::FileMetadataService_CreateFileMetadataEditor(ifc_filemetadata_editor **file_metadata, nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata)
{
	if (IsMyExtension(filename))
	{
		ReferenceCountedObject<MP3MetadataEditor> mp3_metadata;

		if (!mp3_metadata)
			return NErr_OutOfMemory;

		int ret = mp3_metadata->Initialize(filename, file, parent_metadata);
		if (ret != NErr_Success)
			return ret;

		*file_metadata = mp3_metadata;
		mp3_metadata->Retain();
		return NErr_Success;
	}

	return NErr_False;
}

/* ---------- */

ns_error_t MP3FileMetadata::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata *parent_metadata)
{
	ns_error_t ret = MetadataChain<GioFile>::Open(filename, file);
	if (ret != NErr_Success)
	{
		return ret;
	}
	MetadataChain<GioFile>::Close();
	
	return MetadataChain<GioFile>::SetParentMetadata(parent_metadata);

}

/* ---------- */

/* benski>
this simple implementation doesn't handle any metadata writes at all (leaving it all to the underlying parent metadata object)
if we wanted to put Replay Gain values into the LAME tag, then we would need to add a layer here */

MP3MetadataEditor::MP3MetadataEditor()
{
	metadata=0;
}

MP3MetadataEditor::~MP3MetadataEditor()
{
	if (metadata)
		metadata->Release();
	metadata=0;
}

ns_error_t MP3MetadataEditor::Initialize(nx_uri_t filename, nx_file_t file, ifc_metadata_editor *parent_metadata)
{
	metadata = parent_metadata;
	metadata->Retain();
	return NErr_Success;
}

ns_error_t MP3MetadataEditor::FileMetadata_GetMetdataObject(ifc_metadata_editor **out_metadata)
{
	metadata->Retain();
	*out_metadata = metadata;
	return NErr_Success;
}

ns_error_t MP3MetadataEditor::FileMetadata_Save(nx_file_t file)
{
	return NErr_NotImplemented;
}

ns_error_t MP3MetadataEditor::FileMetadata_RequireTempFile()
{
	return NErr_False;
}

ns_error_t MP3MetadataEditor::FileMetadata_SaveAs(nx_file_t destination, nx_file_t source)
{
	return NErr_NotImplemented;
}

ns_error_t MP3MetadataEditor::FileMetadata_WantID3v2(int *position)
{
	*position = ifc_filemetadata_editor::TAG_POSITION_INDIFFERENT;
	return NErr_True;
}

ns_error_t MP3MetadataEditor::FileMetadata_WantID3v1()
{
	return NErr_True;
}

ns_error_t MP3MetadataEditor::FileMetadata_WantAPEv2(int *position)
{
	return NErr_False;
}

ns_error_t MP3MetadataEditor::FileMetadata_WantLyrics3()
{
	return NErr_False;
}