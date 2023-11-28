#include "api.h"
#include "main.h"
#include "FLACMetadataService.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/AutoCharNX.h"
#include "crt/FLACMetadataCallbacks.h"
#include "nu/SafeSize.h"

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

FLACMetadataEditor::FLACMetadataEditor()
{
	itr=0;
	chain=0;
	filename=0;
}

int FLACMetadataEditor::Internal_Initialize(nx_uri_t filename, FLAC__Metadata_Chain *chain, FLAC__Metadata_Iterator *itr)
{
	this->filename = NXURIRetain(filename);
	this->itr = itr;
	this->chain = chain;
	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		FLAC__MetadataType type=FLAC__metadata_iterator_get_block_type(itr);
		FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
		switch (type)
		{
		case FLAC__METADATA_TYPE_VORBIS_COMMENT:
			metadata_block = block;
			break;
		case FLAC__METADATA_TYPE_STREAMINFO:
			stream_info = block;
			break;
		default:
		case FLAC__METADATA_TYPE_PICTURE:
			pictures.push_back(block);
			break;
		}
		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}
	return NErr_Success;
}

FLACMetadataEditor::~FLACMetadataEditor()
{
	if (chain)
		FLAC__metadata_chain_delete(chain);
	if (itr)
		FLAC__metadata_iterator_delete(itr);
	NXURIRelease(filename);
}

int FLACMetadataEditor::Initialize(nx_uri_t filename, nx_file_t file, bool optimize)
{
	FLAC__Metadata_Chain *chain=FLAC__metadata_chain_new();
	if (!chain)
		return NErr_OutOfMemory;

	FLAC__Metadata_Iterator *itr = FLAC__metadata_iterator_new();
	if (!itr)
	{
		FLAC__metadata_chain_delete(chain);
		return NErr_OutOfMemory;
	}

	MetadataReader reader(file);
	FLAC__bool success = FLAC__metadata_chain_read_with_callbacks(chain, &reader, nxfile_io_callbacks);
	reader.Close();

	if (!success)
	{
		FLAC__Metadata_ChainStatus status = FLAC__metadata_chain_status(chain);
		FLAC__metadata_chain_delete(chain);
		FLAC__metadata_iterator_delete(itr);
		switch(status)
		{
		case FLAC__METADATA_CHAIN_STATUS_ERROR_OPENING_FILE:
			return NErr_FileNotFound;
		default:
			return NErr_Error;
		}
	}
	
	if (optimize)
	{
		FLAC__metadata_chain_sort_padding(chain);
		FLAC__metadata_chain_merge_padding(chain);
	}

	ns_error_t ret = Internal_Initialize(filename, chain, itr);

	if (ret != NErr_Success)
	{
		FLAC__metadata_chain_delete(chain);
		FLAC__metadata_iterator_delete(itr);
		return ret;       
	}

	SetFileStats(reader.GetFileStats());
	
	return NErr_Success;
}

static FLAC__StreamMetadata *GetOrMakePadding(FLAC__Metadata_Chain *chain, FLAC__Metadata_Iterator *itr)
{
	FLAC__metadata_iterator_init(itr, chain);
	while (1)
	{
		if (FLAC__METADATA_TYPE_PADDING == FLAC__metadata_iterator_get_block_type(itr))
		{
			FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
			return block;
		}

		if (FLAC__metadata_iterator_next(itr) == false)
			break;
	}
	FLAC__StreamMetadata *padding = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING);
	if (padding)
		FLAC__metadata_iterator_insert_block_after(itr, padding);

	return padding;
}

bool FLACMetadataEditor::NeedsTempFile()
{
	if (FLAC__metadata_chain_check_if_tempfile_needed(chain, true))
		return true;
	else
		return false;
}

ns_error_t FLACMetadataEditor::Internal_Save(nx_file_t destination)
{
	MetadataReader writer(destination);
	FLAC__bool res = FLAC__metadata_chain_write_with_callbacks(chain, true, &writer, nxfile_io_callbacks);
	if (!res)
		return NErr_Error;			
	else
		return NErr_Success;
}

ns_error_t FLACMetadataEditor::Internal_SaveAs(nx_file_t destination, nx_file_t source)
{
	// since we needed to write a tempfile, let's add some more padding so it doesn't happen again
	FLAC__metadata_chain_sort_padding(chain);

	FLAC__StreamMetadata *padding = GetOrMakePadding(chain, itr);
	if (padding)
	{
		// round up to near 4k
		padding->length += 4095;
		padding->length &= ~4096;
	}
	
	MetadataReader reader(source);
	MetadataReader writer(destination);
				
	FLAC__bool res = FLAC__metadata_chain_write_with_callbacks_and_tempfile(chain, false, &reader, nxfile_io_callbacks, &writer, nxfile_io_callbacks);
	reader.Close();
	writer.Close();

	if (!res)
		return NErr_Error;
	else
		return NErr_Success;
}

int FLACMetadataEditor::EraseMetadata(const char *tag, unsigned int index, nx_string_t value)
{
	if (!metadata_block)
		return NErr_Success;
	int ret;
	do
	{
		int pos;
		ret = GetPosition(tag, index, &pos);
		if (ret == NErr_Success)
		{
			FLAC__metadata_object_vorbiscomment_delete_comment(metadata_block, pos);
		}
	} while (ret == NErr_Success);
	return NErr_Success;
}

int FLACMetadataEditor::SetMetadata(const char *tag, unsigned int index, nx_string_t value)
{
	if (!value)
		return EraseMetadata(tag, index, value);

	if (!metadata_block)
	{
		FLAC__metadata_iterator_init(itr, chain);
		do
		{
			if (FLAC__METADATA_TYPE_VORBIS_COMMENT == FLAC__metadata_iterator_get_block_type(itr))
			{
				metadata_block = FLAC__metadata_iterator_get_block(itr);
				break;
			}
		}
		while (FLAC__metadata_iterator_next(itr) != 0);
		if (!metadata_block)
		{
			metadata_block = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT);
			FLAC__metadata_iterator_insert_block_after(itr, metadata_block);
		}		
	}

	if (!metadata_block)
		return NErr_Empty;

	int ret;
	FLAC__StreamMetadata_VorbisComment_Entry entry;

	size_t value_length;
	ret = NXStringGetBytesSize(&value_length, value, nx_charset_utf8, 0);
	if (ret != NErr_Success && ret != NErr_DirectPointer)
		return ret;

	size_t key_length = strlen(tag);
	SafeSize total_length;
	total_length.Add(key_length);
	total_length.Add(1); /* = */
	total_length.Add(value_length);
	total_length.Add(1); /* null terminator */;

	if (total_length.Overflowed())
		return NErr_IntegerOverflow;

	char *comment_string = (char *)malloc(total_length);
	if (!comment_string)
		return NErr_OutOfMemory;

	entry.entry = (FLAC__byte *)comment_string;

	entry.length = total_length;
	memcpy(comment_string, tag, key_length);
	comment_string[key_length] = '=';
	NXStringGetBytes(&value_length, value, comment_string + key_length + 1, value_length, nx_charset_utf8, 0);
	comment_string[key_length + 1 + value_length]=0;

	int pos;
	ret = GetPosition(tag, index, &pos);
	if (ret == NErr_Empty || ret == NErr_EndOfEnumeration)
	{
		//new comment
		if (FLAC__metadata_object_vorbiscomment_append_comment(metadata_block, entry, false) == false)
		{
			free(comment_string);
			return NErr_Error;
		}

	}
	else
	{
		if (FLAC__metadata_object_vorbiscomment_set_comment(metadata_block, pos, entry, false) == false)
		{
			free(comment_string);
			return NErr_Error;
		}
	}

	return NErr_Success;
}

int FLACMetadataEditor::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	int ret;
	switch (field)
	{
	case MetadataKeys::TITLE:
		return SetMetadata("TITLE", index, value);
	case MetadataKeys::ARTIST:
		return SetMetadata("ARTIST", index, value);
	case MetadataKeys::ALBUM:
		return SetMetadata("ALBUM", index, value);
	case MetadataKeys::ALBUM_ARTIST:
		ret = SetMetadata("ALBUM ARTIST", index, value);
		SetMetadata("ALBUM ARTIST", 0, 0);
		if (value == 0)
			SetMetadata("ENSEMBLE", 0, 0);
		return ret;

	case MetadataKeys::COMMENT:
		return SetMetadata("COMMENT", index, value);
	case MetadataKeys::COMPOSER:
		return SetMetadata("COMPOSER", index, value);
	case MetadataKeys::TRACK:
		ret = SetMetadata("TRACKNUMBER", index, value); /* TODO: fallback to other fields */
		SetMetadata("TRACK", 0, 0);
		return ret;
	case MetadataKeys::TRACK_GAIN:
		return SetMetadata("REPLAYGAIN_TRACK_GAIN", index, value);
	case MetadataKeys::TRACK_PEAK:
		return SetMetadata("REPLAYGAIN_TRACK_PEAK", index, value);
	case MetadataKeys::ALBUM_GAIN:
		return SetMetadata("REPLAYGAIN_ALBUM_GAIN", index, value);
	case MetadataKeys::ALBUM_PEAK:
		return SetMetadata("REPLAYGAIN_ALBUM_PEAK", index, value);
	case MetadataKeys::YEAR:
		ret = SetMetadata("DATE", index, value);
		SetMetadata("YEAR", 0, 0);
		return ret;
	case MetadataKeys::GENRE: 
		return SetMetadata("GENRE", index, value);
	case MetadataKeys::DISC:
		return SetMetadata("DISCNUMBER", index, value);
	case MetadataKeys::BPM:
		return SetMetadata("BPM", index, value);
	case MetadataKeys::PUBLISHER:
		return SetMetadata("PUBLISHER", index, value);
	}

	if (field == MetadataKey_GracenoteFileID)
		return SetMetadata("GracenoteFileID", index, value);
	else if (field == MetadataKey_GracenoteExtData)
		return SetMetadata("GracenoteExtData", index, value);

	return NErr_Unknown;
}

int FLACMetadataEditor::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	return NErr_NotImplemented;
}

int FLACMetadataEditor::MetadataEditor_SetReal(int field, unsigned int index, double value)
{
	return NErr_NotImplemented;
}

static int SetPicture(FLAC__StreamMetadata *block, artwork_t *artwork, data_flags_t flags)
{

	int ret;
	FLAC__StreamMetadata_Picture &picture = block->data.picture;

	const void *picture_data;
	size_t picture_len;
	ret = NXDataGet(artwork->data, &picture_data, &picture_len);
	if (ret != NErr_Success)
		return ret;

	FLAC__metadata_object_picture_set_data(block, (FLAC__byte *)picture_data, picture_len, true);
	picture.width = artwork->width;
	picture.height = artwork->height;
	picture.depth = 32;
	picture.colors = 0;

	ReferenceCountedNXString description;
	if (TestFlag(flags, DATA_FLAG_DESCRIPTION) && NXDataGetDescription(artwork->data, &description) == NErr_Success)
	{
		AutoCharUTF8 description_string;

		ret = description_string.Set(description);
		if (ret != NErr_Success)
			return ret;

		FLAC__metadata_object_picture_set_description(block, (FLAC__byte *)(const char *)description_string, false);
	}
	else
	{
		FLAC__metadata_object_picture_set_description(block, (FLAC__byte *)"", true);// TODO?
	}

	ReferenceCountedNXString mime_type;
	if (NXDataGetMIME(artwork->data, &mime_type) == NErr_Success)
	{
		AutoCharNX<nx_charset_ascii> mime_string;

		ret = mime_string.Set(mime_type);
		if (ret != NErr_Success)
			return ret;

		FLAC__metadata_object_picture_set_mime_type(block, (char *)(const char *)mime_string, false);
	}
	else
	{
		FLAC__metadata_object_picture_set_mime_type(block, "", true);
	}

	return NErr_Success;
}

int FLACMetadataEditor::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;

	FLAC__StreamMetadata_Picture_Type picture_type = FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER; /* TODO */
	if (artwork && artwork->data)
	{
		FLAC__metadata_iterator_init(itr, chain);
		while (1)
		{
			if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
			{
				FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
				FLAC__StreamMetadata_Picture &picture = block->data.picture;
				if (picture.type == picture_type) 
				{
					if (index == 0)
					{
						return SetPicture(block, artwork, flags);
					}
					else
					{
						index--;
					}
				}
			}

			if (FLAC__metadata_iterator_next(itr) == false)
				break;
		}

		// not found. let's add it
		FLAC__StreamMetadata *newBlock = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PICTURE);
		if (!newBlock)
			return NErr_OutOfMemory;
		newBlock->data.picture.type = picture_type;

		int ret = SetPicture(newBlock, artwork, flags);
		if (ret != NErr_Success)
			return ret;
		FLAC__metadata_iterator_insert_block_after(itr, newBlock);
		pictures.push_back(newBlock);
		return ret;
	}
	else
	{
		/* erasing */
		if (!metadata_block)
			return NErr_Success;

		FLAC__metadata_iterator_init(itr, chain);
		while (1)
		{
			if (FLAC__METADATA_TYPE_PICTURE == FLAC__metadata_iterator_get_block_type(itr))
			{
				FLAC__StreamMetadata *block = FLAC__metadata_iterator_get_block(itr);
				FLAC__StreamMetadata_Picture &picture = block->data.picture;
				if (picture.type == picture_type)
				{
					if (index == 0)
					{
						FLAC__metadata_iterator_delete_block(itr, false);
						pictures.erase(block);
					}
					else
					{
						index--;
					}
				}
			}

			if (FLAC__metadata_iterator_next(itr) == false)
				break;
		}
		return NErr_Success;
	}
}