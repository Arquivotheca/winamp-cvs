#include "api.h"
#include "MP4MetadataEditor.h"
#include "main.h"
#include "nswasabi/AutoCharNX.h"
#include "nu/ByteWriter.h"
#include "nsmp4.h"
#include "nswasabi/ReferenceCounted.h"

static inline bool TestFlag(int flags, int flag_to_check)
{
	if (flags & flag_to_check)
		return true;
	return false;
}

int Metadata_iTunes_GetSet(MP4FileHandle mp4_file, unsigned int index, const char *name, uint16_t *item, uint16_t *items);

MP4MetadataEditor::MP4MetadataEditor()
{
	mp4_file=0;
	filename=0;
}

MP4MetadataEditor::~MP4MetadataEditor()
{
	if (mp4_file)
		MP4Close(mp4_file);
	mp4_file=0;
	REPLICANT_API_FILELOCK->UnlockFile(filename);
	NXURIRelease(filename);
	filename=0;
}

int MP4MetadataEditor::Initialize(nx_uri_t filename)
{
	this->filename = NXURIRetain(filename);
	REPLICANT_API_FILELOCK->WaitForWrite(filename);
	mp4_file = MP4Modify(filename);
	if (!mp4_file)
	{
		return NErr_Error;
	}

	return NErr_Success;
}

int MP4MetadataEditor::MetadataEditor_Save()
{
	if (mp4_file)
		MP4Close(mp4_file);
	mp4_file=0;
	MP4Optimize(filename);
	return NErr_Success;
}

static int Metadata_iTunes_DeleteFromKey(MP4FileHandle mp4_file, unsigned int index, const char *key)
{
	nsmp4_metadata_itunes_atom_t atom;
	while (NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, key, index, &atom) == NErr_Success)
	{
		NSMP4_Metadata_iTunes_DeleteAtom(mp4_file, atom);
	}
	return NErr_Success;
}

static int Metadata_iTunes_SetStringFromKey(MP4FileHandle mp4_file, unsigned int index, const char *key, nx_string_t value)
{
	if (value)
	{

		nsmp4_metadata_itunes_atom_t atom;
		int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, key, index, &atom);
		if (ret != NErr_Success)
		{
			ret = NSMP4_Metadata_iTunes_NewKey(mp4_file, key, &atom, nsmp4_metadata_itunes_type_utf8);
			if (ret != NErr_Success)
				return ret;
		}

		return NSMP4_Metadata_iTunes_SetString(mp4_file, atom, value);
	}
	else
	{
		return Metadata_iTunes_DeleteFromKey(mp4_file, index, key);
	}
}

static int Metadata_iTunes_SetUnsignedFromKey(MP4FileHandle mp4_file, unsigned int index, const char *key, uint64_t value, size_t byte_count)
{
	if (value)
	{
		nsmp4_metadata_itunes_atom_t atom;
		int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, key, index, &atom);
		if (ret != NErr_Success)
		{
			ret = NSMP4_Metadata_iTunes_NewKey(mp4_file, key, &atom, nsmp4_metadata_itunes_type_unsigned_integer_be);
			if (ret != NErr_Success)
				return ret;
		}

		return NSMP4_Metadata_iTunes_SetUnsigned(mp4_file, atom, value, byte_count);
	}
	else
	{
		return Metadata_iTunes_DeleteFromKey(mp4_file, index, key);
	}
}

static int Metadata_iTunes_SetStringFromFreeform(MP4FileHandle mp4_file, unsigned int index, const char *name, const char *mean, nx_string_t value)
{
	if (value)
	{
		if (index > 0) /* for now, until we figure out how multiple values work */
			return NErr_Success;
		nsmp4_metadata_itunes_atom_t atom;
		int ret = NSMP4_Metadata_iTunes_FindFreeform(mp4_file, name, mean, &atom);
		if (ret != NErr_Success)
		{
			ret = NSMP4_Metadata_iTunes_NewFreeform(mp4_file, name, mean, &atom, nsmp4_metadata_itunes_type_utf8);
			if (ret != NErr_Success)
				return ret;
		}

		return NSMP4_Metadata_iTunes_SetString(mp4_file, atom, value);
	}
	else
	{
		nsmp4_metadata_itunes_atom_t atom;
		while (NSMP4_Metadata_iTunes_FindFreeform(mp4_file, name, mean, &atom) == NErr_Success)
		{
			return NSMP4_Metadata_iTunes_DeleteAtom(mp4_file, atom);
		}
		return NErr_Success;
	}
}

static int Metadata_iTunes_SetSet(MP4FileHandle mp4_file, unsigned int index, const char *key, uint16_t item, uint16_t items)
{
	if (index != 0)
		return NErr_Success; 
	nsmp4_metadata_itunes_atom_t atom;

	int ret = NSMP4_Metadata_iTunes_FindKey(mp4_file, key, &atom);
	if (ret != NErr_Success)
	{
		ret = NSMP4_Metadata_iTunes_NewKey(mp4_file, key, &atom, nsmp4_metadata_itunes_type_binary);
		if (ret != NErr_Success)
			return ret;
	}

	uint8_t data[8];
	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, data, 8);

	bytewriter_write_zero_n(&byte_writer, 2);
	bytewriter_write_u16_be(&byte_writer, item);
	bytewriter_write_u16_be(&byte_writer, items);
	bytewriter_write_zero_n(&byte_writer, 2);
	return NSMP4_Metadata_iTunes_SetBinary(mp4_file, atom, data, 8);
}

static int Metadata_iTunes_SetSetFromString(MP4FileHandle mp4_file, unsigned int index, const char *key, nx_string_t value)
{
	if (index != 0)
		return NErr_Success;

	if (value)
	{
		size_t bytes_copied;
		char temp[64];
		int ret = NXStringGetBytes(&bytes_copied, value, temp, 64, nx_charset_ascii, nx_string_get_bytes_size_null_terminate);
		if (ret != NErr_Success)
			return ret;

		uint16_t item = atoi(temp), items = 0;
		const char *items_string = strchr(temp, '/');
		if (items_string) 
			items = atoi(items_string + 1);

		return Metadata_iTunes_SetSet(mp4_file, index, key, item, items);
	}
	else
	{
		return Metadata_iTunes_DeleteFromKey(mp4_file, index, key);
	}
}

int MP4MetadataEditor::MetadataEditor_SetField(int field, unsigned int index, nx_string_t value)
{
	switch(field)
	{
	case MetadataKeys::ARTIST:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_artist, value);
	case MetadataKeys::ALBUM_ARTIST:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_albumartist, value);
	case MetadataKeys::ALBUM:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_album, value);
	case MetadataKeys::TITLE:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_name, value);
	case MetadataKeys::GENRE:
		Metadata_iTunes_DeleteFromKey(mp4_file, index, nsmp4_metadata_itunes_genre_numeric);
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_genre, value);
	case MetadataKeys::YEAR:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_year, value);
	case MetadataKeys::TRACK:
		return Metadata_iTunes_SetSetFromString(mp4_file, index, nsmp4_metadata_itunes_track, value);
	case MetadataKeys::DISC:
		return Metadata_iTunes_SetSetFromString(mp4_file, index, nsmp4_metadata_itunes_disk, value);
	case MetadataKeys::COMPOSER:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_writer, value);
	case MetadataKeys::PUBLISHER:
		return Metadata_iTunes_SetStringFromFreeform(mp4_file, index, "publisher", "com.nullsoft.winamp", value);
	case MetadataKeys::BPM:
		if (!value)
		{
			return Metadata_iTunes_DeleteFromKey(mp4_file, index, nsmp4_metadata_itunes_tempo);
		}
		else
		{
			int bpm;
			int ret = NXStringGetIntegerValue(value, &bpm);
			if (ret != NErr_Success)
				return ret;
			return Metadata_iTunes_SetUnsignedFromKey(mp4_file, index, nsmp4_metadata_itunes_tempo, (uint64_t)bpm, 2);
		}

	case MetadataKeys::COMMENT:
		return Metadata_iTunes_SetStringFromKey(mp4_file, index, nsmp4_metadata_itunes_comment, value);
	}

	if (field == MetadataKey_GracenoteFileID)
	{
		Metadata_iTunes_SetStringFromFreeform(mp4_file, index, "gnid", "com.apple.iTunes", 0); // delete obselete metadata storage scheme
		return Metadata_iTunes_SetStringFromFreeform(mp4_file, index, "gnid", "com.gracenote.cddb", value);
	}
	else if (field == MetadataKey_GracenoteExtData)
	{
		Metadata_iTunes_SetStringFromFreeform(mp4_file, index, "gnxd", "com.apple.iTunes", 0); // delete obselete metadata storage scheme
		return Metadata_iTunes_SetStringFromFreeform(mp4_file, index, "gnxd", "com.gracenote.cddb", value);
	}

	return NErr_Unknown;
}

int MP4MetadataEditor::MetadataEditor_SetInteger(int field, unsigned int index, int64_t value)
{
	int ret;
	uint16_t item=0, items=0;
	switch(field)
	{
	case MetadataKeys::TRACK:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &item, &items);
		if (ret != NErr_Success)
		{
			items=0;
		}

		item = (uint16_t )value;
		return Metadata_iTunes_SetSet(mp4_file, index, nsmp4_metadata_itunes_track, item, items);
	case MetadataKeys::DISC:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &item, &items);
		if (ret != NErr_Success)
		{
			items=0;
		}

		item = (uint16_t )value;
		return Metadata_iTunes_SetSet(mp4_file, index, nsmp4_metadata_itunes_disk, item, items);
	case MetadataKeys::TRACKS:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_track, &item, &items);
		if (ret != NErr_Success)
		{
			item=0;
		}

		items = (uint16_t )value;
		return Metadata_iTunes_SetSet(mp4_file, index, nsmp4_metadata_itunes_track, item, items);
	case MetadataKeys::DISCS:
		ret = Metadata_iTunes_GetSet(mp4_file, index, nsmp4_metadata_itunes_disk, &item, &items);
		if (ret != NErr_Success)
		{
			item=0;
		}

		items = (uint16_t )value;
		return Metadata_iTunes_SetSet(mp4_file, index, nsmp4_metadata_itunes_disk, item, items);
	case MetadataKeys::BPM:
		return Metadata_iTunes_SetUnsignedFromKey(mp4_file, index, nsmp4_metadata_itunes_tempo, value, 2);
	}
	return NErr_NotImplemented;
}

int MP4MetadataEditor::MetadataEditor_SetReal(int field, unsigned int index, double value)
{
	return NErr_NotImplemented;
}

static void GetTypeForMIME(nx_string_t mime_type, uint32_t *type)
{
	if (!mime_type)
		*type = nsmp4_metadata_itunes_type_binary;
	else if (NXStringKeywordCompareWithCString(mime_type, "image/gif") == NErr_True)
		*type = nsmp4_metadata_itunes_type_gif;
	else if (NXStringKeywordCompareWithCString(mime_type, "image/jpeg") == NErr_True || NXStringKeywordCompareWithCString(mime_type, "image/jpg") == NErr_True)
		*type = nsmp4_metadata_itunes_type_jpeg;
	else if (NXStringKeywordCompareWithCString(mime_type, "image/png") == NErr_True)
		*type = nsmp4_metadata_itunes_type_png;
	else if (NXStringKeywordCompareWithCString(mime_type, "image/bmp") == NErr_True)
		*type = nsmp4_metadata_itunes_type_bmp;
	else
		*type = nsmp4_metadata_itunes_type_binary;
}

int MP4MetadataEditor::MetadataEditor_SetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;

	if (artwork && artwork->data)
	{
		bool created=false;
		uint32_t type;
		ReferenceCountedNXString mime_type;
		NXDataGetMIME(artwork->data, &mime_type);
		GetTypeForMIME(mime_type, &type);

		nsmp4_metadata_itunes_atom_t atom;
		int ret = NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, nsmp4_metadata_itunes_cover_art, index, &atom);
		if (ret != NErr_Success)
		{
			/* create it */
			ret = NSMP4_Metadata_iTunes_NewKey(mp4_file, nsmp4_metadata_itunes_cover_art, &atom, type);
			if (ret != NErr_Success)
				return ret;
			created=true;
		}

		const void *picture_data;
		size_t picture_length;
		ret = NXDataGet(artwork->data, &picture_data, &picture_length);
		if (ret != NErr_Success)
			return ret;

		ret = NSMP4_Metadata_iTunes_SetBinary(mp4_file, atom, picture_data, picture_length);
		if (ret != NErr_Success && created)
			NSMP4_Metadata_iTunes_DeleteAtom(mp4_file, atom);
		return created;
	}
	else
	{
		nsmp4_metadata_itunes_atom_t atom;
		while (NSMP4_Metadata_iTunes_EnumerateKey(mp4_file, nsmp4_metadata_itunes_cover_art, index, &atom) == NErr_Success)
		{
			NSMP4_Metadata_iTunes_DeleteAtom(mp4_file, atom);
		}
		return NErr_Success;
	}
}
