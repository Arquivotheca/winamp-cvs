#include "api.h"
#include "nx/nx.h"
#include "MetadataGDO.h"
#include "api_gracenote.h"
#include "metadata/MetadataKeys.h"
#include "metadata/svc_metadata.h"
#include "nswasabi/ReferenceCounted.h"
#include "nu/ByteWriter.h"
#include "nu/ByteReader.h"
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

int GracenoteMetadataService::MetadataService_DeserializeMetadata(nx_data_t data, ifc_metadata **out_metadata)
{
	const void *bytes;
	size_t bytes_length;
	int ret = NXDataGet(data, &bytes, &bytes_length);
	if (ret != NErr_Success)
		return NErr_False;

	if (bytes_length < 17) // need room for at least a GUID and a null terminator
		return NErr_False;

	bytereader_s byte_reader;
	bytereader_init(&byte_reader, bytes, bytes_length);
	GUID serialization_identifier = bytereader_read_uuid_be(&byte_reader);
	if (serialization_identifier == gracenote_metadata_identifier_album)
	{
		gnsdk_gdo_handle_t gdo=0;
		gnsdk_error_t gn_err = gnsdk_manager_gdo_deserialize((gnsdk_cstr_t)bytereader_pointer(&byte_reader), &gdo);
		if (gn_err != GNSDK_SUCCESS)
			return NErr_Error;

		MetadataGDO_AlbumMatch *metadata = new (std::nothrow) MetadataGDO_AlbumMatch;
		if (!metadata)
		{
			gnsdk_manager_gdo_release(gdo);
			return NErr_OutOfMemory;
		}

		ret = metadata->Initialize(gdo);
		gnsdk_manager_gdo_release(gdo);
		if (ret != NErr_Success)
		{
			delete metadata;
			return ret;
		}
		*out_metadata = metadata;
		return NErr_Success;
	}
	else
	{
		return NErr_False;
	}
}

/* === Art Cache === */
static int GetArtOptionForIndex(unsigned int index, gnsdk_cstr_t *option)
{
	switch(index)
	{
	case 0:
		*option = LINK_OPTION_VALUE_IMAGE_SIZE_XLARGE;
		return NErr_Success;
	case 1:
		*option = LINK_OPTION_VALUE_IMAGE_SIZE_LARGE;
		return NErr_Success;
	case 2:
		*option = LINK_OPTION_VALUE_IMAGE_SIZE_MEDIUM;
		return NErr_Success;
	case 3:
		*option = LINK_OPTION_VALUE_IMAGE_SIZE_SMALL;
		return NErr_Success;
	case 4:
		*option = LINK_OPTION_VALUE_IMAGE_SIZE_THUMBNAIL;
		return NErr_Success;
	}
	return NErr_EndOfEnumeration;
}


MetadataGDO_ArtCache::MetadataGDO_ArtCache() : Wasabi2::Dispatchable(0)
{
	memset(cache, 0, sizeof(cache));
	link_handle=0;
	link_failed=false;
}

MetadataGDO_ArtCache::~MetadataGDO_ArtCache()
{
	for (size_t i=0;i<5;i++)
	{
		NXDataRelease(cache[i].art_data);
	}
	if (link_handle)
		gnsdk_link_query_release(link_handle);

}

int MetadataGDO_ArtCache::GetArtDataCache(unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (index >= sizeof(cache)/sizeof(*cache))
		return NErr_False;

	if (cache[index].attempted)
	{
		if (!cache[index].art_data)
			return NErr_Empty;

		if (artwork)
		{
			artwork->data = NXDataRetain(cache[index].art_data);

			/* we don't known width and height, so set them to 0 to indicate that */
			artwork->width=0;
			artwork->height=0;
		}
		return NErr_Success;
	}

	return NErr_False;
}

int MetadataGDO_ArtCache::Add(unsigned int index, gnsdk_byte_t *buffer, gnsdk_size_t buffer_size, gnsdk_cstr_t option)
{
	if (index >= sizeof(cache)/sizeof(*cache))
		return NErr_False;

	nx_data_t data;
	nx_string_t description;

	int ret = NXDataCreate(&data, buffer, buffer_size);
	if (ret != NErr_Success)
	{
		return ret;
	}

	ReferenceCountedNXString mime;
	ret = NXStringCreateWithUTF8(&mime, "image/jpeg");
	if (ret != NErr_Success)
	{
		NXDataRelease(data);
		return ret;
	}

	NXDataSetMIME(data, mime);

	ret = NXStringCreateWithUTF8(&description, option);
	if (ret != NErr_Success)
	{
		NXDataRelease(data);
		NXStringRelease(mime);
		return ret;
	}

	NXDataSetDescription(data, description);

	cache[index].art_data=data;
	return NErr_Success;
}

void MetadataGDO_ArtCache::SetAttempted(unsigned int index)
{
	if (index < sizeof(cache)/sizeof(*cache))
		cache[index].attempted=1;
}

int MetadataGDO_ArtCache::Open(gnsdk_gdo_handle_t gdo)
{
	if (link_failed)
		return NErr_FailedCreate;

	if (link_handle)
		return NErr_Success;

	int ret = REPLICANT_API_GRACENOTE->LinkCreate(&link_handle);
	if (ret != NErr_Success)
		return ret;

	gnsdk_error_t gn_error = gnsdk_link_query_set_gdo(link_handle, gdo);
	if (gn_error != GNSDK_SUCCESS)
	{
		gnsdk_link_query_release(link_handle);
		link_handle=0;
		return NErr_FailedCreate;
	}

	return NErr_Success;
}

int MetadataGDO_ArtCache::Retrieve(gnsdk_gdo_handle_t gdo, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	gnsdk_cstr_t option;
	int ret = GetArtOptionForIndex(index, &option);
	if (ret != NErr_Success)
		return ret;

	ret = GetArtDataCache(index, artwork, flags);
	if (ret != NErr_False)
		return ret;

	ret = Open(gdo);
	if (ret != NErr_Success)
		return ret;

	SetAttempted(index);

	gnsdk_error_t gn_error = gnsdk_link_query_option_set(link_handle, LINK_OPTION_KEY_IMAGE_SIZE, option);

	gnsdk_link_data_type_t data_type;
	gnsdk_byte_t *buffer=0;
	gnsdk_size_t buffer_size=0;
	gn_error = gnsdk_link_query_content_retrieve(link_handle,  gnsdk_link_content_cover_art,  1, &data_type,  &buffer,  &buffer_size);
	if (gn_error == LINKWARN_NotFound)
	{
		return NErr_Empty;
	}

		if (gn_error == GNSDK_SUCCESS)
		{
				ret = Add(index, buffer, buffer_size, option);
				if (ret != NErr_Success)
					return ret;
		
			return GetArtDataCache(index, artwork, flags);
		}
		return NErr_Error;
}

/* === Common === */
MetadataGDO::MetadataGDO()
{
	album_gdo=0;
	art_cache=0;
}

MetadataGDO::~MetadataGDO()
{
	if (album_gdo)
		gnsdk_manager_gdo_release(album_gdo);
	if (art_cache)
		art_cache->Release();
}




int MetadataGDO::Metadata_GetArtwork(int field, unsigned int index, artwork_t *artwork, data_flags_t flags)
{
	if (field != MetadataKeys::ALBUM)
		return NErr_Unknown;

	if (!art_cache)
	{
		art_cache = new (std::nothrow)ReferenceCounted<MetadataGDO_ArtCache>;
		if (!art_cache)
			return NErr_OutOfMemory;
	}
	return art_cache->Retrieve(album_gdo, index, artwork, flags);
}

void MetadataGDO::WriteMetadata(ifc_metadata_editor *metadata, int key)
{
	if (key == -1)
		return;

	for (int index=0;;index++)
	{
		nx_string_t value;
		int ret = Metadata_GetField(key, index, &value);
		if (ret == NErr_Success)
		{
			metadata->SetField(key, index, value);
			NXStringRelease(value);
		}
		else if (ret == NErr_EndOfEnumeration)
		{
			metadata->SetField(key, index, 0);
			break;
		}
		else
			break;

	}
}

/* === Track === */

MetadataGDO_TrackMatch::MetadataGDO_TrackMatch()
{
	track_gdo=0;
}

MetadataGDO_TrackMatch::~MetadataGDO_TrackMatch()
{
	if (track_gdo)
		gnsdk_manager_gdo_release(track_gdo);
}

int MetadataGDO_TrackMatch::Initialize(gnsdk_gdo_handle_t gdo, gnsdk_cstr_t child, unsigned int index, MetadataGDO_ArtCache *art_cache)
{
	if (gnsdk_manager_gdo_child_get(gdo, child, index, &track_gdo) != GNSDK_SUCCESS)
	{
		track_gdo=0;
		return NErr_EndOfEnumeration;
	}

	album_gdo = gdo;
	gnsdk_manager_gdo_addref(album_gdo);

	this->art_cache = art_cache;
	if (art_cache)
		art_cache->Retain();

	return NErr_Success;
}

static int GetGracenoteMetadata(gnsdk_gdo_handle_t gdo, gnsdk_cstr_t field, unsigned int index, nx_string_t *value)
{
	if (!gdo)
		return NErr_Empty;

	gnsdk_cstr_t gn_value;
	gnsdk_error_t error = gnsdk_manager_gdo_value_get(gdo, field, index+1, &gn_value);
	if (error == GNSDK_SUCCESS)
	{
		return NXStringCreateWithUTF8(value, gn_value);
	}
	else if (index == 0)
	{
		return NErr_Empty;
	}
	else
	{
		return NErr_EndOfEnumeration;
	}
}

// returns ##/## for track and disc
static int GetGracenoteMetadataSlash(gnsdk_gdo_handle_t gdo1, gnsdk_cstr_t field1, gnsdk_gdo_handle_t gdo2, gnsdk_cstr_t field2, unsigned int index, nx_string_t *value)
{
	if (!gdo1)
		return NErr_Empty;

	gnsdk_cstr_t gn_value1;
	gnsdk_error_t error = gnsdk_manager_gdo_value_get(gdo1, field1, index+1, &gn_value1);
	if (error == GNSDK_SUCCESS)
	{
		gnsdk_cstr_t gn_value2;
		if (gdo2 && gnsdk_manager_gdo_value_get(gdo2, field2, index+1, &gn_value2) == GNSDK_SUCCESS)
		{
			char temp[128];
#ifdef _WIN32
			_snprintf(temp, 127, "%s/%s", gn_value1, gn_value2);
#else
			snprintf(temp, 127, "%s/%s", gn_value1, gn_value2);
#endif
			temp[127]=0;
			return NXStringCreateWithUTF8(value, temp);
		}
		else /* couldn't get second field, just return first */
			return NXStringCreateWithUTF8(value, gn_value1);
	}
	else if (index == 0)
	{
		return NErr_Empty;
	}
	else
	{
		return NErr_EndOfEnumeration;
	}
}

int MetadataGDO_TrackMatch::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	int ret;
	switch(field)
	{
	case MetadataKeys::ARTIST: 
		ret = GetGracenoteMetadata(track_gdo, GNSDK_GDO_VALUE_ARTIST_DISPLAY, index, value);
		if (ret != NErr_Success)
			return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_ARTIST_DISPLAY, index, value);

	case MetadataKeys::ALBUM_ARTIST: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_ARTIST_DISPLAY, index, value);

	case MetadataKeys::ALBUM: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_TITLE_DISPLAY, index, value);

	case MetadataKeys::TITLE: 
		return GetGracenoteMetadata(track_gdo, GNSDK_GDO_VALUE_TITLE_DISPLAY, index, value);

	case MetadataKeys::URI: 
		return GetGracenoteMetadata(track_gdo, GNSDK_MUSICIDFILE_GDO_VALUE_FILENAME, index, value);

	case MetadataKeys::GENRE: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_GENRE_MICRO, index, value);


	case MetadataKeys::YEAR: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_DATE, index, value);

	case MetadataKeys::TRACK: 
		return GetGracenoteMetadata(track_gdo, GNSDK_GDO_VALUE_TRACK_NUMBER, index, value);

	case MetadataKeys::DISC: 
		return GetGracenoteMetadataSlash(album_gdo, GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET, album_gdo, GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET, index, value);

		// TODO: MetadataKeys::COMPOSER

	case MetadataKeys::PUBLISHER: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_ALBUM_LABEL, index, value);

		// TODO: MetadataKeys::BPM

		// TODO: MetadataKeys::COMMENT

	}

	if (field == MetadataKey_MatchScore)
	{
		return GetGracenoteMetadata(track_gdo, GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_SCORE, index, value);
	}
	else if (field == MetadataKey_GracenoteFileID)
	{
		return GetGracenoteMetadata(track_gdo, GNSDK_GDO_VALUE_PRODUCTID, index, value);
	}
	else if (field == MetadataKey_GracenoteExtData)
	{
		return GetGracenoteMetadata(track_gdo, GNSDK_GDO_VALUE_EXTENDED_DATA, index, value);
	}


	return NErr_Unknown;
}

static int GetGracenoteMetadataInteger(gnsdk_gdo_handle_t gdo, gnsdk_cstr_t field, unsigned int index, int64_t *value)
{
	if (!gdo)
		return NErr_Empty;

	gnsdk_cstr_t gn_value;
	gnsdk_error_t error = gnsdk_manager_gdo_value_get(gdo, field, index+1, &gn_value);
	if (error == GNSDK_SUCCESS)
	{
		*value = strtoul(gn_value, 0, 10);
		return NErr_Success;
	}
	else if (index == 0)
	{
		return NErr_Empty;
	}
	else
	{
		return NErr_EndOfEnumeration;
	}

}
int MetadataGDO_TrackMatch::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	if (field == MetadataKey_MatchScore)
	{
		return GetGracenoteMetadataInteger(track_gdo, GNSDK_MUSICIDFILE_GDO_VALUE_MATCH_SCORE, index, value);
	}
	switch(field)
	{
	case MetadataKeys::TRACK:	
		return GetGracenoteMetadataInteger(track_gdo, GNSDK_GDO_VALUE_TRACK_NUMBER, index, value);
	case MetadataKeys::DISC:
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET, index, value);
	case MetadataKeys::DISCS: 
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET, index, value);
	case MetadataKeys::TRACKS: 
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_TRACK_COUNT, index, value);
	}
	return NErr_Unknown;
}

int MetadataGDO_TrackMatch::Metadata_GetReal(int field, unsigned int index, double *value)
{
	return NErr_NotImplemented;
}

static void CleanFilename(char *ptr, size_t length)
{
	for (size_t i=0;i<length;i++)
	{
		switch(ptr[i])
		{
		case '?':
		case '*':
		case  '|':
			ptr[i] = '_';
			break;
		case '/':
		case '\\':
		case ':':
			ptr[i] =  '-';
			break;
		case '\"': 
			ptr[i]  = '\'';
			break;
		case '<':
			ptr[i]  = '(';
			break;
		case '>':
			ptr[i] = ')';
			break;
		}
	}
}

int MetadataGDO::SaveTo(nx_uri_t filename, int flags)
{
	bool has_existing_embedded_art=false;
	ifc_metadata *reader;
	int ret = REPLICANT_API_METADATA->CreateMetadata(&reader, filename);
	if (ret != NErr_Success)
		reader=0;

	if (reader)
	{
		if (reader->GetArtwork(MetadataKeys::ALBUM, 0, 0) == NErr_Success)
			has_existing_embedded_art=true;
		reader->Release();
	}

	ifc_metadata_editor *metadata;
	ret = REPLICANT_API_METADATA->CreateMetadataEditor(&metadata, filename);
	if (ret != NErr_Success)
		return ret;

	// Check if we want to write metadata
	if ( (flags & GracenoteAPI::SAVE_NO_METADATA) != GracenoteAPI::SAVE_NO_METADATA )
	{
		WriteMetadata(metadata, MetadataKeys::ARTIST);
		WriteMetadata(metadata, MetadataKeys::ALBUM_ARTIST);
		WriteMetadata(metadata, MetadataKeys::ALBUM);
		WriteMetadata(metadata, MetadataKeys::TITLE);
		WriteMetadata(metadata, MetadataKeys::URI);
		WriteMetadata(metadata, MetadataKeys::GENRE);
		WriteMetadata(metadata, MetadataKeys::YEAR);
		WriteMetadata(metadata, MetadataKeys::TRACK);
		WriteMetadata(metadata, MetadataKeys::DISC);
		WriteMetadata(metadata, MetadataKeys::PUBLISHER);
		WriteMetadata(metadata, MetadataKey_GracenoteFileID);
		WriteMetadata(metadata, MetadataKey_GracenoteExtData);
	}

	// Check if we want to write album art
	if ( (flags & GracenoteAPI::SAVE_NO_COVER_ART) != GracenoteAPI::SAVE_NO_COVER_ART )	
	{
		artwork_t artwork;
		for (int art_index=0;art_index<5;art_index++)
		{
			if (Metadata_GetArtwork(MetadataKeys::ALBUM, art_index, &artwork, DATA_FLAG_DATA|DATA_FLAG_MIME) == NErr_Success)
			{
				if ((flags & GracenoteAPI::SAVE_FORCE_EMBED_ART) || has_existing_embedded_art)
				{
					metadata->SetArtwork(MetadataKeys::ALBUM, 0, &artwork, DATA_FLAG_DATA|DATA_FLAG_MIME);
					metadata->SetArtwork(MetadataKeys::ALBUM, 1, 0);
				}
				else
				{
					// write %album%.jpg
					ReferenceCountedNXURI path;
					if (NXURICreateRemovingFilename(&path, filename) == NErr_Success)
					{
						ReferenceCountedNXString album_name;

						if (Metadata_GetField(MetadataKeys::ALBUM, 0, &album_name) == NErr_Success)
						{
							size_t byte_count=0;
							int ret = NXStringGetBytesSize(&byte_count, album_name, nx_charset_utf8, 0);
							if (ret == NErr_DirectPointer || ret == NErr_Success)
							{
								char *ptr = (char *)malloc(byte_count+6); // +1 for prefixed ".",  +4 for ".jpg" and +1 for terminator
								if (ptr)
								{
									ptr[0]='.';
									size_t length;
									NXStringGetBytes(&length, album_name, ptr+1, byte_count, nx_charset_utf8, 0);
									length++; // adjust for prefixed "."
									CleanFilename(ptr, length);									
									ptr[length++]='.';
									ptr[length++]='j';
									ptr[length++]='p';
									ptr[length++]='g';
									ptr[length++]=0;

									ReferenceCountedNXURI album_name_uri;
									if (NXURICreateWithUTF8(&album_name_uri, ptr) == NErr_Success)
									{
										ReferenceCountedNXURI jpeg_filename;
										if (NXURICreateWithPath(&jpeg_filename, album_name_uri, path) == NErr_Success)
										{
											FILE *f = NXFile_fopen(jpeg_filename, nx_file_FILE_write_binary);
											if (f)
											{
												const void *bytes;
												size_t length;
												NXDataGet(artwork.data, &bytes, &length);
												fwrite(bytes, length, 1, f);
												fclose(f);
											}
										}
									}
								}
							}
						}
					}
				}
				break;
			}
		}
	}

	metadata->Save();
	metadata->Release();

	return NErr_Success;
}

/* === Album === */
MetadataGDO_AlbumMatch::MetadataGDO_AlbumMatch()
{

}

MetadataGDO_AlbumMatch::~MetadataGDO_AlbumMatch()
{

}

int MetadataGDO_AlbumMatch::Initialize(gnsdk_gdo_handle_t gdo)
{
	album_gdo = gdo;
	gnsdk_manager_gdo_addref(album_gdo);

	return NErr_Success;
}

int MetadataGDO_AlbumMatch::Metadata_GetField(int field, unsigned int index, nx_string_t *value)
{
	switch(field)
	{
	case MetadataKeys::ALBUM_ARTIST: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_ARTIST_DISPLAY, index, value);

	case MetadataKeys::ALBUM: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_TITLE_DISPLAY, index, value);

	case MetadataKeys::GENRE: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_GENRE_MICRO, index, value);

	case MetadataKeys::YEAR: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_DATE, index, value);

	case MetadataKeys::DISC: 
		return GetGracenoteMetadataSlash(album_gdo, GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET, album_gdo, GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET, index, value);

	case MetadataKeys::PUBLISHER: 
		return GetGracenoteMetadata(album_gdo, GNSDK_GDO_VALUE_ALBUM_LABEL, index, value);
	}
	return NErr_Unknown;
}

int MetadataGDO_AlbumMatch::Metadata_GetInteger(int field, unsigned int index, int64_t *value)
{
	if (field == MetadataKey_MatchedTrack)
	{
		gnsdk_uint32_t count;
		if (gnsdk_manager_gdo_child_count(album_gdo, GNSDK_GDO_CHILD_TRACK_MATCHED, &count) == NErr_Success)
		{
			*value = count;
			return NErr_Success;
		}
		else
		{
			return NErr_Error;
		}		
	}
	switch(field)
	{
	case MetadataKeys::DISC:
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_DISC_IN_SET, index, value);
	case MetadataKeys::DISCS: 
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_TOTAL_IN_SET, index, value);
	case MetadataKeys::TRACKS: 
		return GetGracenoteMetadataInteger(album_gdo, GNSDK_GDO_VALUE_ALBUM_TRACK_COUNT, index, value);
	}
	return NErr_Unknown;
}

int MetadataGDO_AlbumMatch::Metadata_GetReal(int field, unsigned int index, double *value)
{
	return NErr_NotImplemented;
}

int MetadataGDO_AlbumMatch::Metadata_GetMetadata(int type, unsigned int index, ifc_metadata **metadata)
{
	gnsdk_cstr_t child;
	if (type == MetadataKeys::TRACK)
	{
		child=GNSDK_GDO_CHILD_TRACK;

	}
	else if (type == MetadataKey_MatchedTrack)
	{
		child=GNSDK_GDO_CHILD_TRACK_MATCHED;
	}
	else
		return NErr_Unknown;

	MetadataGDO_TrackMatch *track = new (std::nothrow) ReferenceCounted<MetadataGDO_TrackMatch>;
	if (!track)
		return NErr_OutOfMemory;

	if (!art_cache)
		art_cache = new (std::nothrow)ReferenceCounted<MetadataGDO_ArtCache>;

	if (!art_cache)
		return NErr_OutOfMemory;

	int ret = track->Initialize(album_gdo, child, index+1, art_cache);
	if (ret != NErr_Success)
	{
		track->Release();
		return ret;
	}
	*metadata = track;
	return NErr_Success;

}

int MetadataGDO_AlbumMatch::Metadata_Serialize(nx_data_t *out_data)
{
	gnsdk_str_t serialized_data;
	gnsdk_error_t gn_err = gnsdk_manager_gdo_serialize(album_gdo, &serialized_data);
	if (gn_err != GNSDK_SUCCESS)
		return NErr_Error;

	size_t serialized_length = strlen(serialized_data);
	nx_data_t data = 0;
	void *bytes;
	int ret = NXDataCreateWithSize(&data, &bytes, serialized_length+16+1);
	if (ret != NErr_Success)
	{
		gnsdk_manager_string_free(serialized_data);
		return ret;
	}

	bytewriter_s byte_writer;
	bytewriter_init(&byte_writer, bytes, serialized_length+16+1);
	bytewriter_write_uuid_be(&byte_writer, gracenote_metadata_identifier_album);
	bytewriter_write_n(&byte_writer, serialized_data, serialized_length);
	bytewriter_write_u8(&byte_writer, 0); // write a null terminator to make it easier to deserialize directly from an nx_data_t buffer
	gnsdk_manager_string_free(serialized_data);
	*out_data = data;
	return NErr_Success;
}
