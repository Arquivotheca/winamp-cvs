#include "api.h"
#include "FileMetadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/AutoBuffer.h"
#ifdef __ANDROID__
#include <android/log.h>
#endif

FileMetadataWrite::FileMetadataWrite()
{
	editor=0;
}

FileMetadataWrite::~FileMetadataWrite()
{
	if (editor)
		editor->Release();
}

int FileMetadataWrite::Initialize(ifc_filemetadata_editor *editor)
{
	this->editor = editor;
	editor->Retain();
	return NErr_Success;
}

int FileMetadataWrite::MetadataEditor_Save()
{
	bool require_temp_file=false;

	if (editor && editor->RequireTempFile() == NErr_True)
	{
		require_temp_file=true;
	}
	else if (id3v2.tag && id3v2.position == 0)
	{
		// see if we have to "Grow" the beginning of the file
		uint32_t new_id3v2_size=0;
		if (NSID3v2_Tag_SerializedSize(id3v2.tag, &new_id3v2_size, 0, 0) == NErr_Success)
		{
			if (new_id3v2_size > id3v2.length)
			{
				require_temp_file=true;
			}
		}
	}
	
	if (require_temp_file)
	{
		ReferenceCountedNXURI temp;
		int ret = NXURICreateTempForFilepath(&temp, filename);
		if (ret != NErr_Success)
			return ret;

#ifdef __ANDROID__
		__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileMetadataWrite] Saving %s via temp file %s", filename->string, ((nx_uri_t)temp)->string);
#endif
		ret = MetadataEditor_SaveAs(temp);
		if (ret != NErr_Success)
			return ret;

		if (REPLICANT_API_FILELOCK)
			REPLICANT_API_FILELOCK->WaitForWrite(filename);
		ret = NXFile_move(filename, temp);
		if (REPLICANT_API_FILELOCK)
			REPLICANT_API_FILELOCK->UnlockFile(filename);
		if (ret != NErr_Success)
		{
			NXFile_unlink(temp);
			return ret;
		}
		return NErr_Success;		
	}

#ifdef __ANDROID__
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileMetadataWrite] Modifying %s in place", filename->string);
#endif
	
	AutoBuffer temp;
	if (REPLICANT_API_FILELOCK)
		REPLICANT_API_FILELOCK->WaitForWrite(filename);
	nx_file_t fd;
	ns_error_t ret = NXFileOpenFile(&fd, filename, nx_file_FILE_update_binary);
	if (ret != NErr_Success)
	{
		if (REPLICANT_API_FILELOCK)
			REPLICANT_API_FILELOCK->UnlockFile(filename);
		return ret;
	}

	// TODO: compare modified times
	if (id3v2.tag && id3v2.position == 0) /* make sure it's meant to go at the beginning  */
	{
		uint32_t id3v2_size=0;
		if (NSID3v2_Tag_SerializedSize(id3v2.tag, &id3v2_size, id3v2.length, SerializedSize_AbsoluteSize) == NErr_Success)
		{
			int ret = temp.Reserve(id3v2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(fd);
				if (REPLICANT_API_FILELOCK)
					REPLICANT_API_FILELOCK->UnlockFile(filename);
				return ret;
			}

			if (NSID3v2_Tag_Serialize(id3v2.tag, temp, id3v2_size, SerializedSize_AbsoluteSize) == NErr_Success)
			{
				NXFileWrite(fd, temp, id3v2_size);
			}
		}
	}

	// allow the underlying file editor to make any necessary changes
	NXFileLockRegion(fd, start_position, start_position+content_length);
	if (!editor || editor->Save(fd) != NErr_Success)
	{
		NXFileSeek(fd, content_length);
	}
	else
	{
		NXFileLength(fd, &content_length);
		NXFileSeek(fd, content_length);
	}

	if (apev2.tag)
	{
		size_t apev2_size=0;		
		if (NSAPEv2_Tag_SerializedSize(apev2.tag, &apev2_size) == NErr_Success)
		{		
			int ret = temp.Reserve(apev2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(fd);
				if (REPLICANT_API_FILELOCK)
					REPLICANT_API_FILELOCK->UnlockFile(filename);
				return ret;
			}

			if (NSAPEv2_Tag_Serialize(apev2.tag, temp, apev2_size) == NErr_Success)
			{
				NXFileWrite(fd, temp, apev2_size);
			}
		}
	}

	if (lyrics3.tag)
	{
		NXFileWrite(fd, lyrics3.tag, lyrics3.length);
	}

	if (id3v2.tag && id3v2.position != 0) /* make sure it's meant to go at the end */
	{
		uint32_t id3v2_size=0;
		if (NSID3v2_Tag_SerializedSize(id3v2.tag, &id3v2_size, 0, 0) == NErr_Success)
		{
			int ret = temp.Reserve(id3v2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(fd);
				if (REPLICANT_API_FILELOCK)
					REPLICANT_API_FILELOCK->UnlockFile(filename);
				return ret;
			}


			if (NSID3v2_Tag_Serialize(id3v2.tag, temp, id3v2_size, 0) == NErr_Success)
			{
				NXFileWrite(fd, temp, id3v2_size);
			}
		}
	}

	if (id3v1.tag)
	{
		char temp[128];
		if (NSID3v1_Tag_Serialize(id3v1.tag, temp, 128) == NErr_Success)
		{
			NXFileWrite(fd, temp, 128);
		}
	}

#ifdef __ANDROID__
	//__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[FileMetadataWrite] expected position=%llu, actual=%llu", end_position, _lseeki64(fd, 0, SEEK_CUR));
#endif

	NXFileSync(fd);
	NXFileTruncate(fd);

	NXFileRelease(fd);
	if (REPLICANT_API_FILELOCK)
		REPLICANT_API_FILELOCK->UnlockFile(filename);
	return NErr_Success;
}

int FileMetadataWrite::MetadataEditor_SaveAs(nx_uri_t destination)
{
	AutoBuffer temp;

	nx_file_t source, f;

	if (editor)
		editor->Close();

	ns_error_t ret = NXFileOpenFile(&source, filename, nx_file_FILE_read_binary);
	if (ret != NErr_Success)
		return ret;

	NXFileLockRegion(source, start_position, end_position);

	// TODO: compare modified times

	ret = NXFileOpenFile(&f, destination, nx_file_FILE_write_binary);
	if (ret != NErr_Success)
	{
		NXFileRelease(source);
		return ret;
	}

	if (id3v2.tag && id3v2.position == 0) /* make sure it's meant to go at the beginning  */
	{
		uint32_t id3v2_size=0;

		if (NSID3v2_Tag_SerializedSize(id3v2.tag, &id3v2_size, 4096, SerializedSize_BlockSize) == NErr_Success)
		{
			int ret = temp.Reserve(id3v2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(source);
				NXFileRelease(f);
				return ret;
			}

			if (NSID3v2_Tag_Serialize(id3v2.tag, temp, id3v2_size, SerializedSize_BlockSize) == NErr_Success)
			{
				NXFileWrite(f, temp, id3v2_size);
				NXFileLockRegion(f, id3v2_size, id3v2_size);
			}
		}
	}

	
	NXFileSeek(source, 0);
	if (editor && editor->SaveAs(f, source) == NErr_Success)
	{
		uint64_t length;
		// make sure we're at the end of the output file
		NXFileLength(f, &length);
		NXFileSeek(f, length);
	}
	else
	{
		// manual copy
		for (;;)
		{
			uint8_t temp[4096];

			size_t bytes_read;
			ret = NXFileRead(source, temp, sizeof(temp), &bytes_read);
			if (ret == NErr_EndOfFile)
				break;
			else if (ret != NErr_Success)
			{
				NXFileRelease(source);
				NXFileRelease(f);
				return ret;
			}
			NXFileWrite(f, temp, bytes_read);
		}
	}

	NXFileRelease(source);

	if (apev2.tag)
	{
		size_t apev2_size=0;		
		if (NSAPEv2_Tag_SerializedSize(apev2.tag, &apev2_size) == NErr_Success)
		{		
			int ret = temp.Reserve(apev2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(f);
				NXFile_unlink(destination);
				return ret;
			}

			if (NSAPEv2_Tag_Serialize(apev2.tag, temp, apev2_size) == NErr_Success)
			{
				ret = NXFileWrite(f, temp, apev2_size);
				if (ret != NErr_Success)
				{
					NXFileRelease(f);
					NXFile_unlink(destination);
					return ret;
				}
			}
		}
	}

	if (lyrics3.tag)
	{
		ret = NXFileWrite(f, lyrics3.tag, lyrics3.length);
		if (ret != NErr_Success)
		{
			NXFileRelease(f);
			NXFile_unlink(destination);
			return ret;
		}
	}

	if (id3v2.tag && id3v2.position != 0) /* make sure it's meant to go at the end */
	{
		uint32_t id3v2_size=0;
		if (NSID3v2_Tag_SerializedSize(id3v2.tag, &id3v2_size, 0, 0) == NErr_Success)
		{
			int ret = temp.Reserve(id3v2_size);
			if (ret != NErr_Success)
			{
				NXFileRelease(f);
				NXFile_unlink(destination);
				return ret;
			}

			if (NSID3v2_Tag_Serialize(id3v2.tag, temp, id3v2_size, 0) == NErr_Success)
			{
				ret = NXFileWrite(f, temp, id3v2_size);
				if (ret != NErr_Success)
				{
					NXFileRelease(f);
					NXFile_unlink(destination);
					return ret;
				}
			}
		}
	}

	if (id3v1.tag)
	{
		char temp[128];
		if (NSID3v1_Tag_Serialize(id3v1.tag, temp, 128) == NErr_Success)
		{
			ret = NXFileWrite(f, temp, 128);
			if (ret != NErr_Success)
			{
				NXFileRelease(f);
				NXFile_unlink(destination);
				return ret;
			}			
		}
	}

	NXFileRelease(f);
	return NErr_Success;
}


int FileMetadataWrite::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	int ret;
	bool known=false;

	ret = MakeID3v2();
	if (ret != NErr_Success)
		return ret;
	
	if (id3v2.tag)
	{
		ret = id3v2_metadata.MetadataEditor_SetField(field, index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			known=true;
		else if (ret != NErr_Unknown)
			return ret;
	}

	/* make an ID3v1 tag if it doesn't already exist */
	if (!id3v1.tag  && editor && editor->WantID3v1() == NErr_True)
	{
		ret = NSID3v1_Tag_New(&id3v1.tag);
		if (ret != NErr_Success)
			return ret;

		id3v1_metadata.Initialize(id3v1.tag);
	}

	if (id3v1.tag)
	{
		ret = id3v1_metadata.MetadataEditor_SetField(field, index, value);
		if (ret == NErr_Success  || ret == NErr_EndOfEnumeration)
			known=true;
		else if (ret != NErr_Unknown)
			return ret;
	}

	// TODO: make an APEv2 tag if they want one
	if (apev2.tag)
	{
		ret = apev2_metadata.MetadataEditor_SetField(field, index, value);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			known=true;
		else if (ret != NErr_Unknown)
			return ret;
	}

	if (known)
		return NErr_Success;
	else
		return NErr_Empty;
}

int FileMetadataWrite::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	return NErr_NotImplemented;
}

int FileMetadataWrite::MetadataEditor_SetReal(int field, unsigned int index, double value)
{
	return NErr_NotImplemented;
}

int FileMetadataWrite::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	int ret;
	bool known=false;

	ret = MakeID3v2();
	if (ret != NErr_Success)
		return ret;

	
	if (id3v2.tag)
	{
		ret = id3v2_metadata.MetadataEditor_SetArtwork(field, index, artwork, flags);
		if (ret == NErr_Success || ret == NErr_EndOfEnumeration)
			known=true;
		else if (ret != NErr_Unknown)
			return ret;
	}

	// TODO: make an APEv2 tag if they want one

	if (apev2.tag)
	{
		/* for now, only write APEv2 artwork if there was a pre-existing one */
		if (apev2_metadata.Metadata_GetArtwork(field, 0, 0, DATA_FLAG_NONE) == NErr_Success)
			return apev2_metadata.MetadataEditor_SetArtwork(field, index, artwork, flags);
	}
	if (known)
		return NErr_Success;
	else
		return NErr_Empty;
}

int FileMetadataWrite::MakeID3v2()
{
	ns_error_t ret;
	int tag_position;
		/* make an ID3v2 tag if it doesn't already exist */
	if (!id3v2.tag && editor && editor->WantID3v2(&tag_position) == NErr_True)
	{
		nsid3v2_header_t new_header;
		ret = NSID3v2_Header_New(&new_header, 3, 0);
		if (ret != NErr_Success)
			return ret;

#if 0 // TODO
		if (tag_position == ifc_filemetadata_editor::TAG_POSITION_APPENDED)
		{
			// TODO: add footer, etc
			id3v2.position = 1; // just some non-zero value to force it at the end
		}
#endif

		ret = NSID3v2_Tag_Create(&id3v2.tag, new_header, 0, 0);
		NSID3v2_Header_Destroy(new_header);
		if (ret != NErr_Success)
			return ret;

		id3v2_metadata.Initialize(id3v2.tag);
	}
	return NErr_Success;
}