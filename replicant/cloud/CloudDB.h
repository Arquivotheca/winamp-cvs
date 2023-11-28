#pragma once
#include "sqlite/sqlite3.h"
#include "nx/nxstring.h"
#include "nx/nxuri.h"
#include "ifc_clouddb.h"
#include "CloudDB-Helper.h"
#include "Attributes.h"
#ifdef _WIN32
#include "../../nu/AutoChar.h" // TODO: benski> I don't like this one bit but it'll work for now
#endif
#include "nswasabi/AutoCharNX.h"
#include "AutoNormalize.h"
#include "sha1.h"

class Cloud_Statement
{
public:
	Cloud_Statement()
	{
		statement=0;
	}

	~Cloud_Statement()
	{
		if (statement)
		{
			sqlite3_reset(statement);
			sqlite3_finalize(statement);
		}
	}

	sqlite3_stmt **operator &()
	{
		return &statement;
	}

	operator sqlite3_stmt *&()
	{
		return statement;
	}

private:
	sqlite3_stmt *statement;
};

class AutoResetStatement
{
public:
	AutoResetStatement(sqlite3_stmt *statement) : statement(statement)
	{
	}
	~AutoResetStatement()
	{
		if (statement)
			sqlite3_reset(statement);
	}
private:
	sqlite3_stmt *statement;
};

class Cloud_DBConnection : public ifc_clouddb
{
public:
	static int CreateConnection(Cloud_DBConnection **out_db, nx_uri_t database_path, nx_uri_t settings_path, nx_string_t device_token);
	~Cloud_DBConnection();	

	int WASABICALL CloudDB_BeginTransaction();
	int WASABICALL CloudDB_Commit();
	int WASABICALL CloudDB_Reset_All();
	int WASABICALL CloudDB_Compact();
	int WASABICALL CloudDB_Rollback();

	int WASABICALL CloudDB_Attribute_Add(const char *attribute_name, int *attribute_id);

	int WASABICALL CloudDB_Info_SetDeviceName(nx_string_t device_name);

	int Song_AddAttributeData(int song_id, int attribute_id, nx_string_t data);

	int WASABICALL CloudDB_Media_Delete(int internal_id);
	int WASABICALL CloudDB_Media_FindByFilename(nx_uri_t filename, int device_id, int *internal_id, int *is_ignored);
	int WASABICALL CloudDB_Media_FindFilepathByMediahash(int attribute_id, nx_string_t mediahash, nx_uri_t *value);
	int WASABICALL CloudDB_Media_FindFilepathByMetahash(int attribute_id, nx_string_t metahash, nx_uri_t *value);

	int WASABICALL CloudDB_Media_Add(nx_uri_t filename, ifc_metadata *metadata, int dirty_flags, int *internal_id);
	int WASABICALL CloudDB_Media_Update(int internal_id, ifc_metadata *metadata, int dirty_flags);
	int Media_ClearAttribute(int internal_id, int attribute_id);

#pragma region idmap
	// TODO: benski> IDMap setting and retrieval are getting to be a bit cumbersome. 
	// There's a lot of fields.  We should consider using db/ifc_cursor as a return value, and let users get whatever fields they want out
	int IDMap_SetProperties(int media_id, int64_t playcount, int64_t lastplayed, int64_t lastupdated, int64_t filetime, int64_t filesize, int64_t bitrate, double duration);
	int WASABICALL CloudDB_IDMap_GetProperties(int media_id, int64_t *playcount, int64_t *lastplayed, int64_t *lastupdated, int64_t *filetime, int64_t *filesize, int64_t *bitrate, double *duration);
	int WASABICALL CloudDB_IDMap_SetPlayedProperties(int media_id, int64_t playcount, int64_t lastplayed);
	int WASABICALL CloudDB_IDMap_GetPlayedProperties(int media_id, int64_t *playcount, int64_t *lastplayed);
	int WASABICALL CloudDB_IDMap_Get_IDs_From_MetaHash(nx_string_t metahash, int64_t **out_ids, int **out_device_ids, size_t *num_ids);
	int IDMap_Generate(int *internal_id, int device_id, nx_uri_t filename);
	int IDMap_Associate(int *internal_id, int cloud_id, int device_id, nx_uri_t filename);
	int WASABICALL CloudDB_IDMap_Find(int64_t cloud_id, int *internal_id);
	int IDMap_Set(int internal_id, int64_t cloud_id);
	int IDMap_Next(int64_t *cloud_id);
	int WASABICALL CloudDB_IDMap_Get(int internal_id, int64_t *cloud_id);
	int WASABICALL CloudDB_IDMap_Get_Filepath(int internal_id, nx_uri_t *filepath);
	int WASABICALL CloudDB_IDMap_Remove(int internal_id);
	int WASABICALL CloudDB_IDMap_Delete(int internal_id);
	int IDMap_Removed(int internal_id);
	int WASABICALL CloudDB_IDMap_GetMetaHash(int internal_id, nx_string_t *metahash);
	int WASABICALL CloudDB_IDMap_GetMediaHash(int internal_id, nx_string_t *mediahash);
	int WASABICALL CloudDB_IDMap_SetMediaHash(int internal_id, nx_string_t mediahash);
	int IDMap_Get_Unannounced(int *internal_id, ns_error_t first);
	int IDMap_Get_To_Remove(int **internal_ids, size_t *num_ids);
	int WASABICALL CloudDB_IDMap_Get_MediaHash_Null(int **internal_ids, size_t *num_ids);
	int WASABICALL CloudDB_IDMap_Get_Devices_From_MediaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids);
	int WASABICALL CloudDB_IDMap_Get_Devices_Token_From_MediaHash(nx_string_t mediahash, int **out_device_ids, nx_string_t **out_tokens, size_t *num_device_ids);
	int WASABICALL CloudDB_IDMap_Get_Devices_From_MetaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids);
	int WASABICALL CloudDB_IDMap_SetMetaHash(int internal_id, nx_string_t metahash);
	int IDMap_SetDirty(int internal_id, int dirty);
	int WASABICALL CloudDB_IDMap_AddDirty(int internal_id, int dirty);
	int WASABICALL CloudDB_IDMap_ResetDirty();
	int IDMap_Get_Dirty(int dirty_flag, int **internal_ids, int **dirties, size_t *num_ids);
	int WASABICALL CloudDB_IDMap_SetIgnore(int internal_id);
	int WASABICALL CloudDB_IDMap_ResetIgnored(int internal_id);
	int WASABICALL CloudDB_IDMap_ResetIgnored();
	int WASABICALL CloudDB_IDMap_Get_Ignored(int **internal_ids, nx_string_t **out_filenames, size_t *num_ids);
	int WASABICALL CloudDB_IDMap_GetSyncFilePaths(int source_device_id, int dest_device_id, nx_string_t **out_filepaths, size_t *num_filepaths);
	int WASABICALL CloudDB_IDMap_GetDeviceSizeSum(int device_id, int64_t *device_size);
	int WASABICALL CloudDB_IDMap_GetDeviceCloudFiles(int device_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids);
	int WASABICALL CloudDB_IDMap_GetDeviceNameFromFilepath(nx_uri_t filepath, nx_string_t **out_devicenames, size_t *num_names);
	int WASABICALL CloudDB_IDMap_GetDeviceNameFromMetahash(nx_string_t metahash, nx_string_t **out_devicenames, size_t *num_names);
	int IDMap_SetAlbumHash(int internal_id, nx_string_t albumhash);
	int WASABICALL CloudDB_IDMap_GetMIME(int internal_id, nx_string_t *mime_type);
	int IDMap_SetMIME(int internal_id, nx_string_t mime_type);
	int IDMap_SetTitle(int internal_id, nx_string_t title);
	int WASABICALL CloudDB_IDMap_GetTitle(int internal_id, nx_string_t *title);
	int IDMap_SetIDHash(int internal_id, nx_string_t idhash);
	int IDMap_GetIDHash(int internal_id, nx_string_t *idhash);

	
	int WASABICALL CloudDB_IDMap_GetString(int internal_id, const char *column, nx_string_t *value);
	int WASABICALL CloudDB_IDMap_GetInteger(int internal_id, const char *column, int64_t *value);
	int WASABICALL CloudDB_IDMap_SetString(int internal_id, const char *column, nx_string_t value);
	int WASABICALL CloudDB_IDMap_SetInteger(int internal_id, const char *column, int64_t value);
#pragma endregion
	int Info_Populate(nx_string_t device_id);
	int Info_IncrementRevision();
	int WASABICALL CloudDB_Info_GetRevision(int64_t *revision);
	int Info_GetRevisionID(nx_string_t *revision_id);
	int Info_SetRevisionID(nx_string_t revision_id);
	int WASABICALL CloudDB_Info_SetRevision(int64_t revision);
	int WASABICALL CloudDB_Info_GetLogging(int *revision);
	int WASABICALL CloudDB_Info_SetLogging(int revision);
	int Info_GetDeviceName(nx_string_t *device_name);
	int Info_SetDeviceName(nx_string_t device_name);

	int WASABICALL CloudDB_Playlists_AddUpdate(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int64_t created, int dirty, int* mode);
	int WASABICALL CloudDB_Playlists_Update(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int dirty);
	int WASABICALL CloudDB_Playlists_Remove(nx_string_t uuid);
	int WASABICALL CloudDB_Playlists_Removed(nx_string_t uuid);
	int WASABICALL CloudDB_Playlists_SetDirty(nx_string_t uuid, int dirty);
	int WASABICALL CloudDB_Playlists_Get(nx_string_t uuid, int64_t *playlist_id, nx_string_t *name, double *duration, int64_t *entries, int64_t *lastupdated, int64_t *created);
	int WASABICALL CloudDB_Playlists_GetLastUpdate(nx_string_t uuid, int64_t *lastupdated);
	int WASABICALL CloudDB_Playlists_Find(nx_string_t uuid, int64_t *playlist_id, int64_t *dirty);
	int WASABICALL CloudDB_Playlist_GetIDs(nx_string_t **uuids, int64_t **playlist_ids, size_t *num_playlists);
	int Playlists_Get_Dirty(nx_string_t **uuids, int64_t **playlist_ids, int **dirties, size_t *num_playlists);
	int WASABICALL CloudDB_Playlists_GetID(nx_string_t uuid, int64_t *id);
	int WASABICALL CloudDB_Playlists_SetID(nx_string_t uuid, int64_t id);

	int WASABICALL CloudDB_PlaylistMap_GetMetahash(nx_string_t uuid, int item, nx_string_t *metahash);

	int WASABICALL CloudDB_MetahashMap_GetMetadata(MetadataMap *metadata, size_t *num_metahash);

	int WASABICALL CloudDB_Media_GetIDs(int device_id, int **ids, size_t *num_ids);

	int WASABICALL CloudDB_Devices_GetIDs(nx_string_t **devices, size_t *num_devices);
	int WASABICALL CloudDB_Devices_GetDeviceIDs(int **device_ids, size_t *num_devices);
	int WASABICALL CloudDB_Devices_Add(nx_string_t device_token, nx_string_t friendly_name, DeviceInfoStruct *device_info, int *device_id);
	int WASABICALL CloudDB_Devices_Remove(int device_id);
	int WASABICALL CloudDB_Devices_Find(nx_string_t device_token, int *device_id, DeviceInfoStruct *device_info);
	int WASABICALL CloudDB_Devices_GetName(int device_id, nx_string_t *name, nx_string_t *device_token);
	int WASABICALL CloudDB_Devices_GetCapacity(int device_id, int64_t *total_size, int64_t *used_size);
	int WASABICALL CloudDB_Devices_StoreCapacity(int device_id, int64_t total_size, int64_t used_size);
	int WASABICALL CloudDB_Devices_GetLastSeen(int device_id, int64_t *last_seen, int *on);
	int WASABICALL CloudDB_Devices_SetLastSeen(int device_id, int64_t last_seen, int on);
	int Devices_SetAvailability(int device_id, int64_t transient, double availability);
	int Devices_SetLocal(int device_id);
	int Devices_SetLAN(int device_id, int lan);
	int Devices_ResetLAN();

	int WASABICALL CloudDB_MIME_SetPlayable(nx_string_t mime_type, int playable, int streamable);

	int WASABICALL CloudDB_Artwork_GetWork(int device_id, int attribute_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids);
	int Artwork_Associate(int internal_id, int attribute_id, nx_string_t arthash, nx_uri_t source_uri, int64_t filetime, int64_t last_modified);
	int Artwork_Get(int internal_id, nx_string_t *arthash);

	int WriterBlocks();
	int ComputePlaylistHash(nx_string_t playlist, nx_string_t *playlisthash);

protected:
	Cloud_DBConnection();
	int Initialize(nx_uri_t database_path, nx_string_t device_token);
	int ComputeMetaHash(int internal_id, nx_string_t *metahash);
	int ComputeAlbumHash(int internal_id, nx_string_t *albumhash);
	int ComputeIDHash(int internal_id, nx_string_t *idhash);
	void Internal_Compute(nx_string_t value, SHA1_CTX *sha1);

#pragma region Add/Update Internal Helpers
	enum
	{
		FALLBACK_NONE=0,
		FALLBACK_REAL=1, // try to get the value as a real if GetInteger fails
		FALLBACK_NOW=2, // use the current time if GetInteger fails
		FALLBACK_INTEGER=3, // fall back to integer if string fails
	};
	void UpdateString(int internal_id, const char *column, ifc_metadata *metadata, int field, int fallback=FALLBACK_NONE);
	void UpdateReal(int internal_id, const char *column, ifc_metadata *metadata, int field);	
	void UpdateInteger(int internal_id, const char *column, ifc_metadata *metadata, int field, int fallback=FALLBACK_NONE);
#pragma endregion

	int PrepareStatement(sqlite3_stmt *&statement, const char *sql, size_t sql_cch);

	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch);

	template <class _t>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t value)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1, _t2 value2)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2, class _t3>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1, _t2 value2, _t3 value3)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 3, value3);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2, class _t3, class _t4>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1, _t2 value2, _t3 value3, _t4 value4)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 3, value3);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 4, value4);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2, class _t3, class _t4, class _t5>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1, _t2 value2, _t3 value3, _t4 value4, _t5 value5)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 3, value3);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 4, value4);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 5, value5);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2, class _t3, class _t4, class _t5, class _t6>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1, _t2 value2, _t3 value3, _t4 value4, _t5 value5, _t6 value6)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 3, value3);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 4, value4);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 5, value5);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 6, value6);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t1, class _t2, class _t3, class _t4, class _t5, class _t6, class _t7>
	int Step(sqlite3_stmt *&statement, const char *sql, size_t sql_cch, _t1 value1,
			 _t2 value2, _t3 value3, _t4 value4, _t5 value5, _t6 value6, _t7 value7)
	{
		int sqlite_ret=PrepareStatement(statement, sql, sql_cch);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 1, value1);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 2, value2);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 3, value3);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 4, value4);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 5, value5);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 6, value6);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret=sqlite3_bind_any(statement, 7, value7);
		if (sqlite_ret != SQLITE_OK)
			return sqlite_ret;

		sqlite_ret =  sqlite3_step(statement);	
		if (sqlite_ret == SQLITE_BUSY)
		{
			sqlite_ret=sqlite_ret;
		}
		return sqlite_ret;
	}

	template <class _t>
	void Columns(sqlite3_stmt *statement, _t *value)
	{
		sqlite3_column_any(statement, 0, value);		
		sqlite3_reset(statement);
	}

	template <class _t1, class _t2>
	void Columns(sqlite3_stmt *statement, _t1 *value1, _t2 *value2)
	{
		sqlite3_column_any(statement, 0, value1);
		sqlite3_column_any(statement, 1, value2);
		sqlite3_reset(statement);
	}

	template <class _t1, class _t2, class _t3>
	void Columns(sqlite3_stmt *statement, _t1 *value1, _t2 *value2, _t3 *value3)
	{
		sqlite3_column_any(statement, 0, value1);
		sqlite3_column_any(statement, 1, value2);
		sqlite3_column_any(statement, 2, value3);
		sqlite3_reset(statement);
	}

	template <class _t1, class _t2, class _t3, class _t4>
	void Columns(sqlite3_stmt *statement, _t1 *value1, _t2 *value2, _t3 *value3, _t4 *value4)
	{
		sqlite3_column_any(statement, 0, value1);
		sqlite3_column_any(statement, 1, value2);
		sqlite3_column_any(statement, 2, value3);
		sqlite3_column_any(statement, 3, value4);
		sqlite3_reset(statement);
	}

	template <class _t1, class _t2, class _t3, class _t4, class _t5>
	void Columns(sqlite3_stmt *statement, _t1 *value1, _t2 *value2, _t3 *value3, _t4 *value4, _t5 *value5)
	{
		sqlite3_column_any(statement, 0, value1);
		sqlite3_column_any(statement, 1, value2);
		sqlite3_column_any(statement, 2, value3);
		sqlite3_column_any(statement, 3, value4);
		sqlite3_column_any(statement, 4, value5);
		sqlite3_reset(statement);
	}

	sqlite3 *database_connection;
	Cloud_Statement statement_begin;
	Cloud_Statement statement_commit;
	Cloud_Statement statement_rollback;

	Cloud_Statement statement_attributes_find;
	Cloud_Statement statement_attributes_add;

	Cloud_Statement statement_values_add;
	Cloud_Statement statement_values_find;

	Cloud_Statement statement_songs_updateattributevalue;

	Cloud_Statement statement_idmap_generate;
	Cloud_Statement statement_idmap_associate;
	Cloud_Statement statement_idmap_set;
	Cloud_Statement statement_idmap_next;
	Cloud_Statement statement_idmap_find;
	Cloud_Statement statement_idmap_get;
	Cloud_Statement statement_idmap_remove;
	Cloud_Statement statement_idmap_delete;
	Cloud_Statement statement_idmap_removed;
	Cloud_Statement statement_idmap_get_metahash;
	Cloud_Statement statement_idmap_get_mediahash;
	Cloud_Statement statement_idmap_set_mediahash;
	Cloud_Statement statement_idmap_get_unannounced;
	Cloud_Statement statement_idmap_get_to_remove;
	Cloud_Statement statement_idmap_get_mediahash_null;
	Cloud_Statement statement_idmap_get_devices_from_mediahash;
	Cloud_Statement statement_idmap_get_devices_token_from_mediahash;
	Cloud_Statement statement_idmap_get_devices_from_metahash;
	Cloud_Statement statement_idmap_set_albumhash;
	Cloud_Statement statement_idmap_get_filepath;
	Cloud_Statement statement_idmap_set_metahash;
	Cloud_Statement statement_idmap_set_ignore;
	Cloud_Statement statement_idmap_get_ignored;
	Cloud_Statement statement_idmap_reset_ignored_one;
	Cloud_Statement statement_idmap_reset_ignored;
	Cloud_Statement statement_idmap_set_dirty;
	Cloud_Statement statement_idmap_add_dirty;
	Cloud_Statement statement_idmap_get_dirty;
	Cloud_Statement statement_idmap_set_properties;
	Cloud_Statement statement_idmap_get_properties;
	Cloud_Statement statement_idmap_set_played_properties;
	Cloud_Statement statement_idmap_get_played_properties;
	Cloud_Statement statement_idmap_get_ids_from_metahash;
	Cloud_Statement statement_idmap_get_sync_filepaths;
	Cloud_Statement statement_idmap_get_devicesizesum;
	Cloud_Statement statement_idmap_get_device_cloud_files;
	Cloud_Statement statement_idmap_get_device_name_from_filepath;
	Cloud_Statement statement_idmap_get_device_name_from_metahash;
	Cloud_Statement statement_mime_types_add_mime;
	Cloud_Statement statement_mime_types_get_mime_id;
	Cloud_Statement statement_mime_types_add_playable;
	Cloud_Statement statement_mime_types_set_playable;
	Cloud_Statement statement_idmap_get_mime;
	Cloud_Statement statement_idmap_set_mime;
	Cloud_Statement statement_idmap_get_title;
	Cloud_Statement statement_idmap_set_title;
	Cloud_Statement statement_idmap_get_idhash;
	Cloud_Statement statement_idmap_set_idhash;

	Cloud_Statement statement_metahashmap_get_metadata;

	Cloud_Statement statement_idmap_get_for_metahash;
	Cloud_Statement statement_idmap_get_for_albumhash;

	Cloud_Statement statement_info_populate;
	Cloud_Statement statement_info_getdevicename;
	Cloud_Statement statement_info_setdevicename;
	Cloud_Statement statement_info_incrementrevision;
	Cloud_Statement statement_info_getrevision;
	Cloud_Statement statement_info_setrevision;
	Cloud_Statement statement_info_getrevisionid;
	Cloud_Statement statement_info_setrevisionid;
	Cloud_Statement statement_info_getlogging;
	Cloud_Statement statement_info_setlogging;

	Cloud_Statement statement_media_get_ids_for_device;
	Cloud_Statement statement_media_find_filepath_by_mediahash;
	Cloud_Statement statement_media_find_filepath_by_metahash;
	Cloud_Statement statement_media_find_by_filename;

	Cloud_Statement statement_devices_add;
	Cloud_Statement statement_devices_remove;
	Cloud_Statement statement_devices_find;
	Cloud_Statement statement_devices_find_full;
	Cloud_Statement statement_devices_update;
	Cloud_Statement statement_devices_update_extra;
	Cloud_Statement statement_devices_list;
	Cloud_Statement statement_device_ids_list;
	Cloud_Statement statement_devices_getname;
	Cloud_Statement statement_devices_get_capacity;
	Cloud_Statement statement_devices_save_capacity;
	Cloud_Statement statement_devices_get_lastseen;
	Cloud_Statement statement_devices_set_lastseen;
	Cloud_Statement statement_devices_set_availability;
	Cloud_Statement statement_devices_set_local;
	Cloud_Statement statement_devices_set_lan;
	Cloud_Statement statement_devices_reset_lan;

	Cloud_Statement statement_playlists_add;
	Cloud_Statement statement_playlists_update;
	Cloud_Statement statement_playlists_remove;
	Cloud_Statement statement_playlists_removed;
	Cloud_Statement statement_playlists_setdirty;
	Cloud_Statement statement_playlists_get;
	Cloud_Statement statement_playlists_get_lastupdate;
	Cloud_Statement statement_playlists_find;
	Cloud_Statement statement_playlists_list;
	Cloud_Statement statement_playlists_get_dirty;
	Cloud_Statement statement_playlists_getid;
	Cloud_Statement statement_playlists_setid;

	Cloud_Statement statement_playlistmap_get_metahash;

	Cloud_Statement statement_devicemap_list;

	Cloud_Statement statement_artwork_get_uncalculated;
	Cloud_Statement statement_artwork_associate;
	Cloud_Statement statement_artwork_get;

	Attributes attributes;
	size_t transaction_iterator;
	bool wal_mode;

#ifdef _WIN32
	AutoNormalize normalizer;
	AutoCharGrow converter;
#endif
};