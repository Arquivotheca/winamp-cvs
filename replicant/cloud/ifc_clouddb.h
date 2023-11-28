#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "metadata/ifc_metadata.h"
#include "cb_cloudevents.h"
#include "../nu/vector.h"

class ifc_clouddb : public Wasabi2::Dispatchable
{
protected:
	ifc_clouddb() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_clouddb() {}
public:
	enum
	{
		DIRTY_NONE = 0,
		DIRTY_LOCAL = (1 << 0),
		DIRTY_FULL = (1 << 1),
		DIRTY_REMOTE = (1 << 2),
	};
	enum
	{
		PLAYLIST_DONE = 0,
		PLAYLIST_LOCAL_ADD = (1 << 0),
		PLAYLIST_LOCAL_UPDATE = (1 << 1),
		PLAYLIST_REMOTE = (1 << 2),
		PLAYLIST_REMOVE = (1 << 3),
	};

	int BeginTransaction() { return CloudDB_BeginTransaction(); }
	int Commit() { return CloudDB_Commit(); }
	int Reset_All() { return CloudDB_Reset_All(); }
	int Compact() { return CloudDB_Compact(); }
	int Rollback() { return CloudDB_Rollback(); }

	int IDMap_Find(int64_t cloud_id, int *internal_id) { return CloudDB_IDMap_Find(cloud_id, internal_id); }
	int IDMap_Get(int internal_id, int64_t *cloud_id) { return CloudDB_IDMap_Get(internal_id, cloud_id); }
	int IDMap_GetMetaHash(int internal_id, nx_string_t *metahash) { return CloudDB_IDMap_GetMetaHash(internal_id, metahash); }
	int IDMap_GetMediaHash(int internal_id, nx_string_t *mediahash) { return CloudDB_IDMap_GetMediaHash(internal_id, mediahash); }
	int IDMap_Get_MediaHash_Null(int **internal_ids, size_t *num_ids) { return CloudDB_IDMap_Get_MediaHash_Null(internal_ids, num_ids); }
	int IDMap_Get_Devices_From_MediaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)
		{ return CloudDB_IDMap_Get_Devices_From_MediaHash(mediahash, out_device_ids, num_device_ids, out_internal_ids); }
	int IDMap_Get_Devices_Token_From_MediaHash(nx_string_t mediahash, int **out_device_ids, nx_string_t **out_tokens, size_t *num_device_ids)
		{ return CloudDB_IDMap_Get_Devices_Token_From_MediaHash(mediahash, out_device_ids, out_tokens, num_device_ids); }
	int IDMap_Get_Devices_From_MetaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)
		{ return CloudDB_IDMap_Get_Devices_From_MetaHash(mediahash, out_device_ids, num_device_ids, out_internal_ids); }
	int IDMap_Get_Filepath(int internal_id, nx_uri_t *filepath) { return CloudDB_IDMap_Get_Filepath(internal_id, filepath); }
	int IDMap_Get_IDs_From_MetaHash(nx_string_t metahash, int64_t **out_ids, int **out_device_ids, size_t *num_ids) { return CloudDB_IDMap_Get_IDs_From_MetaHash(metahash, out_ids, out_device_ids, num_ids); }
	int IDMap_GetTitle(int internal_id, nx_string_t *title) { return CloudDB_IDMap_GetTitle(internal_id, title); }

	int IDMap_AddDirty(int internal_id, int dirty_flags) { return CloudDB_IDMap_AddDirty(internal_id, dirty_flags); }
	
	int IDMap_Remove(int internal_id) { return CloudDB_IDMap_Remove(internal_id); }
	int IDMap_Delete(int internal_id) { return CloudDB_IDMap_Delete(internal_id); }
	int IDMap_SetIgnore(int internal_id) { return CloudDB_IDMap_SetIgnore(internal_id); }
	int IDMap_ResetIgnored(int internal_id) { return CloudDB_IDMap_ResetIgnored(internal_id); }
	int IDMap_ResetIgnored() { return CloudDB_IDMap_ResetIgnored(); }
	int IDMap_Get_Ignored(int **internal_ids, nx_string_t **filenames, size_t *num_ids) { return CloudDB_IDMap_Get_Ignored(internal_ids, filenames, num_ids); }

	int IDMap_GetProperties(int media_id, int64_t *playcount, int64_t *lastplayed, int64_t *lastupdated, int64_t *filetime, int64_t *filesize, int64_t *bitrate, double *duration)
		{ return CloudDB_IDMap_GetProperties(media_id, playcount, lastplayed, lastupdated, filetime, filesize, bitrate, duration); }
	int IDMap_SetPlayedProperties(int media_id, int64_t playcount, int64_t lastplayed)
		{ return CloudDB_IDMap_SetPlayedProperties(media_id, playcount, lastplayed); }
	int IDMap_GetPlayedProperties(int media_id, int64_t *playcount, int64_t *lastplayed)
		{ return CloudDB_IDMap_GetPlayedProperties(media_id, playcount, lastplayed); }
	int IDMap_GetSyncFilePaths(int source_device_id, int dest_device_id, nx_string_t **out_filepaths, size_t *num_filepaths)
		{ return CloudDB_IDMap_GetSyncFilePaths(source_device_id, dest_device_id, out_filepaths, num_filepaths); }
	int IDMap_GetDeviceSizeSum(int device_id, int64_t *device_size)
		{ return CloudDB_IDMap_GetDeviceSizeSum(device_id, device_size); }
	int IDMap_GetDeviceCloudFiles(int device_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids)
		{ return CloudDB_IDMap_GetDeviceCloudFiles(device_id, out_filenames, out_ids, num_ids); }
	int IDMap_GetDeviceNameFromFilepath(nx_uri_t filepath, nx_string_t **out_devicenames, size_t *num_names)
		{ return CloudDB_IDMap_GetDeviceNameFromFilepath(filepath, out_devicenames, num_names); }
	int IDMap_GetDeviceNameFromMetahash(nx_string_t metahash, nx_string_t **out_devicenames, size_t *num_names)
		{ return CloudDB_IDMap_GetDeviceNameFromMetahash(metahash, out_devicenames, num_names); }
	int IDMap_GetMIME(int internal_id, nx_string_t *mime_type) { return CloudDB_IDMap_GetMIME(internal_id, mime_type); }
	int IDMap_GetString(int internal_id, const char *column, nx_string_t *value) { return CloudDB_IDMap_GetString(internal_id, column, value); }
	int IDMap_GetInteger(int internal_id, const char *column, int64_t *value) { return CloudDB_IDMap_GetInteger(internal_id, column, value); }
	int IDMap_SetString(int internal_id, const char *column, nx_string_t value) { return CloudDB_IDMap_SetString(internal_id, column, value); }
	int IDMap_SetInteger(int internal_id, const char *column, int64_t value) { return CloudDB_IDMap_SetInteger(internal_id, column, value); }

	typedef Vector<ifc_metadata*> MetadataMap;
	int MetahashMap_GetMetadata(MetadataMap *metadata, size_t *num_metahash) { return CloudDB_MetahashMap_GetMetadata(metadata, num_metahash); }

	int Media_GetIDs(int device_id, int **media_ids, size_t *num_ids) { return CloudDB_Media_GetIDs(device_id, media_ids, num_ids); }
	int Media_FindByFilename(nx_uri_t filename, int device_id, int *internal_id, int *is_ignored) { return CloudDB_Media_FindByFilename(filename, device_id, internal_id, is_ignored); }
	int Media_FindFilepathByMediahash(int device_id, nx_string_t mediahash, nx_uri_t *value) { return CloudDB_Media_FindFilepathByMediahash(device_id, mediahash, value); }
	int Media_FindFilepathByMetahash(int device_id, nx_string_t metahash, nx_uri_t *value) { return CloudDB_Media_FindFilepathByMetahash(device_id, metahash, value); }
	int Media_Add(nx_uri_t filename, ifc_metadata *metadata, int dirty_flags, int *internal_id) { return CloudDB_Media_Add(filename, metadata, dirty_flags, internal_id); }
	int Media_Update(int internal_id, ifc_metadata *metadata, int dirty_flags) { return CloudDB_Media_Update(internal_id, metadata, dirty_flags); }
	
	int Attribute_Add(const char *attribute_name, int *attribute_id) { return CloudDB_Attribute_Add(attribute_name, attribute_id); }

	int Info_SetDeviceName(nx_string_t device_name) { return CloudDB_Info_SetDeviceName(device_name); }
	int Info_SetRevision(int64_t revision) { return CloudDB_Info_SetRevision(revision); }
	int Info_GetRevision(int64_t *revision) { return CloudDB_Info_GetRevision(revision); }	
	int Info_SetLogging(int logging) { return CloudDB_Info_SetLogging(logging); }
	int Info_GetLogging(int *logging) { return CloudDB_Info_GetLogging(logging); }

	int Devices_GetIDs(nx_string_t **devices, size_t *num_devices) { return CloudDB_Devices_GetIDs(devices, num_devices); }
	int Devices_GetDeviceIDs(int **device_ids, size_t *num_devices) { return CloudDB_Devices_GetDeviceIDs(device_ids, num_devices); }
	int Devices_Find(nx_string_t device_token, int *device_id, DeviceInfoStruct *device_info) { return CloudDB_Devices_Find(device_token, device_id, device_info); }
	int Devices_Add(nx_string_t device_token, nx_string_t friendly_name, DeviceInfoStruct *device_info, int *device_id) { return CloudDB_Devices_Add(device_token, friendly_name, device_info, device_id); }
	int Devices_Remove(int device_id) { return CloudDB_Devices_Remove(device_id); }
	int Devices_GetName(int device_id, nx_string_t *name, nx_string_t *device_token) { return CloudDB_Devices_GetName(device_id, name, device_token); }
	int Devices_GetCapacity(int device_id, int64_t *total_size, int64_t *used_size) { return CloudDB_Devices_GetCapacity(device_id, total_size, used_size); }
	int Devices_StoreCapacity(int device_id, int64_t total_size, int64_t used_size) { return CloudDB_Devices_StoreCapacity(device_id, total_size, used_size); }
	int Devices_GetLastSeen(int device_id, int64_t *last_seen, int *on) { return CloudDB_Devices_GetLastSeen(device_id, last_seen, on); }
	int Devices_SetLastSeen(int device_id, int64_t last_seen, int on) { return CloudDB_Devices_SetLastSeen(device_id, last_seen, on); }	

	int Artwork_GetWork(int device_id, int attribute_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids) { return CloudDB_Artwork_GetWork(device_id, attribute_id, out_filenames, out_ids, num_ids); }

	int Playlists_AddUpdate(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int64_t created, int dirty, int* mode)
		{ return CloudDB_Playlists_AddUpdate(uuid, name, duration, entries, lastupdated, created, dirty, mode); }
	int Playlists_Update(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int dirty)
		{ return CloudDB_Playlists_Update(uuid, name, duration, entries, lastupdated, dirty); }
	int Playlists_Remove(nx_string_t uuid) { return CloudDB_Playlists_Remove(uuid); }
	int Playlists_Removed(nx_string_t uuid) { return CloudDB_Playlists_Removed(uuid); }
	int Playlists_SetDirty(nx_string_t uuid, int dirty) { return CloudDB_Playlists_SetDirty(uuid, dirty); }
	int Playlists_Get(nx_string_t uuid, int64_t *playlist_id, nx_string_t *name, double *duration, int64_t *entries, int64_t *lastupdated, int64_t *created)
		{ return CloudDB_Playlists_Get(uuid, playlist_id, name, duration, entries, lastupdated, created); }
	int Playlists_GetLastUpdate(nx_string_t uuid, int64_t *lastupdated) { return CloudDB_Playlists_GetLastUpdate(uuid, lastupdated); }
	int Playlists_Find(nx_string_t uuid, int64_t *playlist_id, int64_t *dirty) { return CloudDB_Playlists_Find(uuid, playlist_id, dirty); }
	int Playlist_GetIDs(nx_string_t **uuids, int64_t **playlist_ids, size_t *num_playlists)
		{ return CloudDB_Playlist_GetIDs(uuids, playlist_ids, num_playlists); }
	int Playlists_GetID(nx_string_t uuid, int64_t *id) { return CloudDB_Playlists_GetID(uuid, id); }
	int Playlists_SetID(nx_string_t uuid, int64_t id) { return CloudDB_Playlists_SetID(uuid, id); }

	int PlaylistMap_GetMetahash(nx_string_t uuid, int item, nx_string_t *metahash) { return CloudDB_PlaylistMap_GetMetahash(uuid, item, metahash); }

	int MIME_SetPlayable(nx_string_t mime_type, int playable, int streamable) { return CloudDB_MIME_SetPlayable(mime_type, playable, streamable); }
private:
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
	virtual int WASABICALL CloudDB_BeginTransaction()=0;
	virtual int WASABICALL CloudDB_Commit()=0;
	virtual int WASABICALL CloudDB_Reset_All()=0;
	virtual int WASABICALL CloudDB_Compact()=0;
	virtual int WASABICALL CloudDB_Rollback()=0;

	virtual int WASABICALL CloudDB_IDMap_Find(int64_t cloud_id, int *internal_id)=0;
	virtual int WASABICALL CloudDB_IDMap_Get(int internal_id, int64_t *cloud_id)=0;
	virtual int WASABICALL CloudDB_IDMap_GetMetaHash(int internal_id, nx_string_t *metahash)=0;
	virtual int WASABICALL CloudDB_IDMap_GetMediaHash(int internal_id, nx_string_t *mediahash)=0;
	virtual int WASABICALL CloudDB_IDMap_GetTitle(int internal_id, nx_string_t *title)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_MediaHash_Null(int **internal_ids, size_t *num_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_Devices_From_MediaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_Devices_Token_From_MediaHash(nx_string_t mediahash, int **out_device_ids, nx_string_t **out_tokens, size_t *num_device_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_Devices_From_MetaHash(nx_string_t mediahash, int **out_device_ids, size_t *num_device_ids, int **out_internal_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_Filepath(int internal_id, nx_uri_t *filepath)=0;
	virtual int WASABICALL CloudDB_IDMap_Delete(int internal_id)=0;
	virtual int WASABICALL CloudDB_IDMap_Remove(int internal_id)=0;
	virtual int WASABICALL CloudDB_IDMap_SetIgnore(int internal_id)=0;

	virtual int WASABICALL CloudDB_IDMap_AddDirty(int internal_id, int dirty)=0;
	virtual int WASABICALL CloudDB_IDMap_ResetIgnored(int internal_id)=0;
	virtual int WASABICALL CloudDB_IDMap_ResetIgnored()=0;
	virtual int WASABICALL CloudDB_IDMap_Get_Ignored(int **internal_ids, nx_string_t **out_filenames, size_t *num_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_GetProperties(int media_id, int64_t *playcount, int64_t *lastplayed, int64_t *lastupdated, int64_t *filetime, int64_t *filesize, int64_t *bitrate, double *duration)=0;
	virtual int WASABICALL CloudDB_IDMap_SetPlayedProperties(int media_id, int64_t playcount, int64_t lastplayed)=0;
	virtual int WASABICALL CloudDB_IDMap_GetPlayedProperties(int media_id, int64_t *playcount, int64_t *lastplayed)=0;
	virtual int WASABICALL CloudDB_IDMap_Get_IDs_From_MetaHash(nx_string_t metahash, int64_t **out_ids, int **out_device_ids, size_t *num_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_GetSyncFilePaths(int source_device_id, int dest_device_id, nx_string_t **out_filepaths, size_t *num_filepaths)=0;
	virtual int WASABICALL CloudDB_IDMap_GetDeviceSizeSum(int device_id, int64_t *device_size)=0;
	virtual int WASABICALL CloudDB_IDMap_GetDeviceCloudFiles(int device_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids)=0;
	virtual int WASABICALL CloudDB_IDMap_GetDeviceNameFromFilepath(nx_uri_t filepath, nx_string_t **out_devicenames, size_t *num_names)=0;
	virtual int WASABICALL CloudDB_IDMap_GetDeviceNameFromMetahash(nx_string_t metahash, nx_string_t **out_devicenames, size_t *num_names)=0;
	virtual int WASABICALL CloudDB_IDMap_GetMIME(int internal_id, nx_string_t *mime_type)=0;
	virtual int WASABICALL CloudDB_IDMap_GetString(int internal_id, const char *column, nx_string_t *value)=0;
	virtual int WASABICALL CloudDB_IDMap_GetInteger(int internal_id, const char *column, int64_t *value)=0;
	virtual int WASABICALL CloudDB_IDMap_SetString(int internal_id, const char *column, nx_string_t value)=0;
	virtual int WASABICALL CloudDB_IDMap_SetInteger(int internal_id, const char *column, int64_t value)=0;

	virtual int WASABICALL CloudDB_MetahashMap_GetMetadata(MetadataMap *metadata, size_t *num_metahash)=0;

	virtual int WASABICALL CloudDB_Media_GetIDs(int device_id, int **media_ids, size_t *num_ids)=0;
	virtual int WASABICALL CloudDB_Media_FindByFilename(nx_uri_t filename, int device_id, int *internal_id, int *is_ignored)=0;
	virtual int WASABICALL CloudDB_Media_FindFilepathByMediahash(int device_id, nx_string_t mediahash, nx_uri_t *value)=0;
	virtual int WASABICALL CloudDB_Media_FindFilepathByMetahash(int device_id, nx_string_t metahash, nx_uri_t *value)=0;
	virtual int WASABICALL CloudDB_Media_Add(nx_uri_t filename, ifc_metadata *metadata, int dirty_flags, int *internal_id)=0;
	virtual int WASABICALL CloudDB_Media_Update(int internal_id, ifc_metadata *metadata, int dirty_flags)=0;

	virtual int WASABICALL CloudDB_Attribute_Add(const char *attribute_name, int *attribute_id)=0;

	virtual int WASABICALL CloudDB_Info_SetDeviceName(nx_string_t device_name)=0;
	virtual int WASABICALL CloudDB_Info_SetRevision(int64_t revision)=0;
	virtual int WASABICALL CloudDB_Info_GetRevision(int64_t *revision)=0;
	virtual int WASABICALL CloudDB_Info_SetLogging(int logging)=0;
	virtual int WASABICALL CloudDB_Info_GetLogging(int *logging)=0;

	virtual int WASABICALL CloudDB_Devices_GetIDs(nx_string_t **devices, size_t *num_devices)=0;
	virtual int WASABICALL CloudDB_Devices_GetDeviceIDs(int **device_ids, size_t *num_devices)=0;
	virtual int WASABICALL CloudDB_Devices_Find(nx_string_t device_token, int *device_id, DeviceInfoStruct *device_info)=0;
	virtual int WASABICALL CloudDB_Devices_Add(nx_string_t device_token, nx_string_t friendly_name, DeviceInfoStruct *device_info, int *device_id)=0;
	virtual int WASABICALL CloudDB_Devices_Remove(int device_id)=0;
	virtual int WASABICALL CloudDB_Devices_GetName(int device_id, nx_string_t *name, nx_string_t *device_token)=0;
	virtual int WASABICALL CloudDB_Devices_GetCapacity(int device_id, int64_t *total_size, int64_t *used_size)=0;
	virtual int WASABICALL CloudDB_Devices_StoreCapacity(int device_id, int64_t total_size, int64_t used_size)=0;
	virtual int WASABICALL CloudDB_Devices_GetLastSeen(int device_id, int64_t *last_seen, int *on)=0;
	virtual int WASABICALL CloudDB_Devices_SetLastSeen(int device_id, int64_t last_seen, int on)=0;

	virtual int WASABICALL CloudDB_Artwork_GetWork(int device_id, int attribute_id, nx_string_t **out_filenames, int **out_ids, size_t *num_ids)=0;

	virtual int WASABICALL CloudDB_Playlists_AddUpdate(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int64_t created, int dirty, int* mode)=0;
	virtual int WASABICALL CloudDB_Playlists_Update(nx_string_t uuid, nx_string_t name, double duration, int64_t entries, int64_t lastupdated, int dirty)=0;
	virtual int WASABICALL CloudDB_Playlists_Remove(nx_string_t uuid)=0;
	virtual int WASABICALL CloudDB_Playlists_Removed(nx_string_t uuid)=0;
	virtual int WASABICALL CloudDB_Playlists_SetDirty(nx_string_t uuid, int dirty)=0;
	virtual int WASABICALL CloudDB_Playlists_Get(nx_string_t uuid, int64_t *playlist_id, nx_string_t *name, double *duration, int64_t *entries, int64_t *lastupdated, int64_t *created)=0;
	virtual int WASABICALL CloudDB_Playlists_GetLastUpdate(nx_string_t uuid, int64_t *lastupdated)=0;
	virtual int WASABICALL CloudDB_Playlists_Find(nx_string_t uuid, int64_t *playlist_id, int64_t *dirty)=0;
	virtual int WASABICALL CloudDB_Playlist_GetIDs(nx_string_t **uuids, int64_t **playlist_ids, size_t *num_playlists)=0;
	virtual int WASABICALL CloudDB_Playlists_GetID(nx_string_t uuid, int64_t *id)=0;
	virtual int WASABICALL CloudDB_Playlists_SetID(nx_string_t uuid, int64_t id)=0;

	virtual int WASABICALL CloudDB_PlaylistMap_GetMetahash(nx_string_t uuid, int item, nx_string_t *metahash)=0;

	virtual int WASABICALL CloudDB_MIME_SetPlayable(nx_string_t mime_type, int playable, int streamable)=0;
	
};