#include "api.h"
#include "main.h"
#include "CloudDB.h"
#include "nu/vector.h"
#include "CloudAPI.h"
#include "nswasabi/ReferenceCounted.h"
#include <stdlib.h>
#include <time.h>

extern CloudAPI cloud_api;
#define SQLPARAM(x) x, sizeof(x)

static const char sql_media_find_by_filename[] = 
"SELECT [idmap].media_id, [idmap].ignore "
"FROM [idmap] "
"WHERE [idmap].device_id=? AND [idmap].filepath=?";
static const char sql_media_find_filepath_by_mediahash[] = "SELECT [idmap].filepath FROM [idmap] WHERE [idmap].device_id=? AND [idmap].mediahash=? AND [idmap].ignore=0";
static const char sql_media_find_filepath_by_metahash[] = "SELECT [idmap].filepath FROM [idmap] WHERE [idmap].device_id=? AND [idmap].metahash=? AND [idmap].ignore=0";

int Cloud_DBConnection::CloudDB_Media_FindByFilename(nx_uri_t filename, int device_id, int *internal_id, int *is_ignored)
{
	int sqlite_ret;

	*internal_id = *is_ignored = 0;

	sqlite_ret=Step(statement_media_find_by_filename, SQLPARAM(sql_media_find_by_filename), device_id, filename);
	AutoResetStatement auto_reset(statement_media_find_by_filename);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_media_find_by_filename, internal_id, is_ignored);
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

int Cloud_DBConnection::CloudDB_Media_FindFilepathByMediahash(int device_id, nx_string_t mediahash, nx_uri_t *value)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_media_find_filepath_by_mediahash, SQLPARAM(sql_media_find_filepath_by_mediahash), device_id, mediahash);
	AutoResetStatement auto_reset(statement_media_find_filepath_by_mediahash);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_media_find_filepath_by_mediahash, value);
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

int Cloud_DBConnection::CloudDB_Media_FindFilepathByMetahash(int device_id, nx_string_t metahash, nx_uri_t *value)
{
	int sqlite_ret;

	sqlite_ret=Step(statement_media_find_filepath_by_metahash, SQLPARAM(sql_media_find_filepath_by_metahash), device_id, metahash);
	AutoResetStatement auto_reset(statement_media_find_filepath_by_metahash);
	if (sqlite_ret == SQLITE_ROW)
	{
		Columns(statement_media_find_filepath_by_metahash, value);
		return NErr_Success;
	}
	else
		return NErr_Empty;
}

int Cloud_DBConnection::CloudDB_IDMap_SetString(int internal_id, const char *column, nx_string_t value)
{
	Cloud_Statement updater;
	char query[256];
	int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
	int sqlite_ret=Step(updater, query, len+1, value, internal_id);
	return NErr_Success;
}

int Cloud_DBConnection::CloudDB_IDMap_SetInteger(int internal_id, const char *column, int64_t value)
{
	Cloud_Statement updater;
	char query[256];
	int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
	int sqlite_ret;
	if (value == -1)
		sqlite_ret=Step(updater, query, len+1, (nx_string_t)0, internal_id);
	else
		sqlite_ret=Step(updater, query, len+1, value, internal_id);
	return NErr_Success;
}

#pragma region Update
void Cloud_DBConnection::UpdateString(int internal_id, const char *column, ifc_metadata *metadata, int field, int fallback)
{
	Cloud_Statement updater;
	ReferenceCountedNXString value;
	int ret = metadata->GetField(field, 0, &value);

	if (ret != NErr_Success)
	{
		if (fallback == FALLBACK_INTEGER)
		{
			UpdateInteger(internal_id, column, metadata, field);
			return;
		}
	}

	if (ret == NErr_Success || ret == NErr_Empty)
	{
		char query[256];
		int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
		int sqlite_ret=Step(updater, query, len+1, value, internal_id);
	}	
}

void Cloud_DBConnection::UpdateReal(int internal_id, const char *column, ifc_metadata *metadata, int field)
{
	Cloud_Statement updater;
	double value=0;
	int ret = metadata->GetReal(field, 0, &value);
	if (ret == NErr_Success || ret == NErr_Empty)
	{
		char query[256];
		int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
		int sqlite_ret=Step(updater, query, len+1, value, internal_id);
	}
}

void Cloud_DBConnection::UpdateInteger(int internal_id, const char *column, ifc_metadata *metadata, int field, int fallback)
{
	Cloud_Statement updater;
	int64_t value=0;
	int ret = metadata->GetInteger(field, 0, &value);

	if (ret != NErr_Success || value==0)
	{
		if (fallback == FALLBACK_REAL)
		{
			UpdateReal(internal_id, column, metadata, field);
			return;
		}
		else if (fallback == FALLBACK_NOW)
		{
			char query[256];
			int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
			int sqlite_ret=Step(updater, query, len+1, (int64_t)time(0), internal_id);
			return;
		}
	}

	if (ret == NErr_Success || ret == NErr_Empty)
	{
		char query[256];
		int len = sprintf(query, "UPDATE [idmap] SET %s=? WHERE media_id=?", column);
		int sqlite_ret=Step(updater, query, len+1, value, internal_id);
	}	
}

int Cloud_DBConnection::CloudDB_Media_Update(int internal_id, ifc_metadata *metadata, int dirty_flags)
{
	int ret;
	ReferenceCountedNXString metahash, albumhash, mime_type, idhash, arthash;

	UpdateString(internal_id, "mediahash", metadata, MetadataKey_CloudMediaHash);

	UpdateString(internal_id, "artist", metadata, MetadataKeys::ARTIST);
	UpdateString(internal_id, "album", metadata, MetadataKeys::ALBUM);
	UpdateString(internal_id, "albumartist", metadata, MetadataKeys::ALBUM_ARTIST);
	UpdateString(internal_id, "year", metadata, MetadataKeys::YEAR, FALLBACK_INTEGER);
	UpdateString(internal_id, "genre", metadata, MetadataKeys::GENRE);
	UpdateString(internal_id, "category", metadata, MetadataKeys::CATEGORY);
	UpdateString(internal_id, "comment", metadata, MetadataKeys::COMMENT);
	UpdateString(internal_id, "composer", metadata, MetadataKeys::COMPOSER);
	UpdateString(internal_id, "director", metadata, MetadataKeys::DIRECTOR);
	UpdateString(internal_id, "producer", metadata, MetadataKeys::PRODUCER);
	UpdateString(internal_id, "publisher", metadata, MetadataKeys::PUBLISHER);
	UpdateString(internal_id, "albumgain", metadata, MetadataKeys::ALBUM_GAIN);
	UpdateString(internal_id, "trackgain", metadata, MetadataKeys::TRACK_GAIN);

	ret = metadata->GetField(MetadataKeys::MIME_TYPE, 0, &mime_type);
	if (ret == NErr_Success || ret == NErr_Empty)
		IDMap_SetMIME(internal_id, mime_type);

	UpdateString(internal_id, "title", metadata, MetadataKeys::TITLE);

	UpdateInteger(internal_id, "playcount", metadata, MetadataKeys::PLAY_COUNT);
	UpdateInteger(internal_id, "lastplayed", metadata, MetadataKeys::LAST_PLAY);
	UpdateInteger(internal_id, "lastupdated", metadata, MetadataKeys::LAST_UPDATE, FALLBACK_NOW);
	UpdateInteger(internal_id, "filetime", metadata, MetadataKeys::FILE_TIME);
	UpdateInteger(internal_id, "filesize", metadata, MetadataKeys::FILE_SIZE);
	UpdateInteger(internal_id, "bitrate", metadata, MetadataKeys::BITRATE, FALLBACK_REAL);
	UpdateReal(internal_id, "duration", metadata, MetadataKeys::LENGTH);

	UpdateInteger(internal_id, "trackno", metadata, MetadataKeys::TRACK);
	UpdateInteger(internal_id, "tracks", metadata, MetadataKeys::TRACKS);
	UpdateInteger(internal_id, "bpm", metadata, MetadataKeys::BPM);
	UpdateInteger(internal_id, "disc", metadata, MetadataKeys::DISC);
	UpdateInteger(internal_id, "discs", metadata, MetadataKeys::DISCS);
	UpdateInteger(internal_id, "rating", metadata, MetadataKeys::RATING);	
	UpdateInteger(internal_id, "added", metadata, MetadataKeys::ADDED);	

	if (metadata->GetField(MetadataKey_CloudMetaHash, 0, &metahash) == NErr_Success || ComputeMetaHash(internal_id, &metahash) == NErr_Success)
		CloudDB_IDMap_SetMetaHash(internal_id, metahash);

	if (metadata->GetField(MetadataKey_CloudAlbumHash, 0, &albumhash) == NErr_Success || ComputeAlbumHash(internal_id, &albumhash) == NErr_Success)
		IDMap_SetAlbumHash(internal_id, albumhash);

	if (metadata->GetField(MetadataKey_CloudIDHash, 0, &idhash) == NErr_Success || ComputeIDHash(internal_id, &idhash) == NErr_Success)
		IDMap_SetIDHash(internal_id, idhash); 

	// TODO TODO review this - allowed through on NErr_Empty to prevent it spinning
	//			 but not sure what the intention is for handling non-arthash items
	ret = metadata->GetField(MetadataKey_CloudArtHashAlbum, 0, &arthash);
	if (ret == NErr_Success || ret == NErr_Empty)
		Artwork_Associate(internal_id, attributes.album, arthash, 0, 0, 0);

	IDMap_AddDirty(internal_id, dirty_flags);

	return NErr_Success;
}

#pragma endregion

int Cloud_DBConnection::CloudDB_Media_Add(nx_uri_t filename, ifc_metadata *metadata, int dirty_flags, int *internal_id)
{
	ReferenceCountedNXString device_token;
	int device_id;
	int64_t cloud_id = 0;
	*internal_id = 0;

	if (metadata->GetField(MetadataKey_CloudDevice, 0, &device_token) != NErr_Success)
		*(&device_token) = NXStringRetain(attributes.device_token);

	Devices_Add(device_token, 0, 0, &device_id);

	if (metadata->GetInteger(MetadataKey_CloudID, 0, &cloud_id) == NErr_Success) 
	{
		int is_ignored;
		/* already has a cloud ID, let's try to reassociate */
		if (IDMap_Find(cloud_id, internal_id) == NErr_Success)
		{
			/* already in the database, let's treat it as an update */
			int64_t filetime_db, filetime_metadata;
			if (CloudDB_IDMap_GetProperties(*internal_id, 0, 0, 0, &filetime_db, 0, 0, 0) == NErr_Success
				&& metadata->GetInteger(MetadataKeys::FILE_TIME, 0, &filetime_metadata) == NErr_Success
				&& filetime_metadata > filetime_db)
			{
				// update if file timestamp differs
				return CloudDB_Media_Update(*internal_id, metadata, dirty_flags);
			}
			else
			{
				// returning NErr_Success is not ideal appropriate if we did't need to do
				// an add so return this so it's still a success but can be discriminated
				return NErr_NoAction;
			}
		}
		else if (Media_FindByFilename(filename, device_id, internal_id, &is_ignored) == NErr_Success)
		{
			IDMap_Set(*internal_id, (int)cloud_id);
			/* already in the database, let's update */
			int64_t filetime_db, filetime_metadata;
			if (CloudDB_IDMap_GetProperties(*internal_id, 0, 0, 0, &filetime_db, 0, 0, 0) == NErr_Success
				&& metadata->GetInteger(MetadataKeys::FILE_TIME, 0, &filetime_metadata) == NErr_Success
				&& filetime_metadata > filetime_db)
			{
				// update if file timestamp differs
				return CloudDB_Media_Update(*internal_id, metadata, dirty_flags);
			}
			else
			{
				// returning NErr_Success is not ideal appropriate if we did't need to do
				// an add so return this so it's still a success but can be discriminated
				return NErr_NoAction;
			}
		}
		else if (IDMap_Associate(internal_id, (int)cloud_id, device_id, filename) == NErr_Success)
		{
		}
		else
		{
			return NErr_Error;
		}
	}
	else
	{
		/* make sure it's not already in the database */
		int is_ignored = 0;
		int ret = Media_FindByFilename(filename, device_id, internal_id, &is_ignored);
		if (ret == NErr_Success)
		{
			if (is_ignored == 0)
			{
				int64_t filetime_db, filetime_metadata;
				if (CloudDB_IDMap_GetProperties(*internal_id, 0, 0, 0, &filetime_db, 0, 0, 0) == NErr_Success
					&& metadata->GetInteger(MetadataKeys::FILE_TIME, 0, &filetime_metadata) == NErr_Success
					&& filetime_metadata > filetime_db)
				{
					// update if file timestamp differs
					return CloudDB_Media_Update(*internal_id, metadata, dirty_flags);
				}
				else
				{
					// returning NErr_Success is not ideal appropriate if we did't need to do
					// an add so return this so it's still a success but can be discriminated
					return NErr_NoAction;
				}
			}			
		}
		/* TODO: benski> add some additional logic around is_ignored
		if is_ignored == 5, we can re-use the ID
		if is_ignored == 3 or 4, it's going to be tricky because we might be in the midst of talking to the server.  
		*/

		// if not found because it's ignored then we need to abort
		if ((ret == NErr_Empty && is_ignored == 1) || is_ignored != 0)
			return NErr_NoAction;


		// so if we got here then we've not been able to get a match
		// so we'll treat it as a new addition and give it a new id.
		IDMap_Generate(internal_id, device_id, filename);
		if (!*internal_id)
			return NErr_FailedCreate;
	}

	// at this point, an update is the same as an add
	CloudDB_IDMap_SetInteger(*internal_id, "added", time(0));
	return CloudDB_Media_Update(*internal_id, metadata, 0);
}
