#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "nu/PtrDeque.h"
#include "JSON-Tree.h"

class ifc_clouddb;
class ifc_cloudclient;

class cb_cloud_upload: public Wasabi2::Dispatchable
{
protected:
	cb_cloud_upload() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_cloud_upload() {}
public:
	int OnProgress(uint64_t bytes, uint64_t total) { return CloudUploadCallback_OnProgress(bytes, total); }
	void OnFinished(int ret) { return CloudUploadCallback_OnFinished(ret); }
	void OnDownloadFinished(nx_uri_t filename, ifc_clouddb *db_connection, const JSON::Value *playlist)
		{ return CloudUploadCallback_OnDownloadFinished(filename, db_connection, playlist); }
	void OnError(nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field) { return CloudUploadCallback_OnError(action, code, message, field); }
	int IsKilled() { return CloudUploadCallback_IsKilled(); }

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual int WASABICALL CloudUploadCallback_OnProgress(uint64_t bytes, uint64_t total) { return 0; }
	virtual void WASABICALL CloudUploadCallback_OnFinished(int ret) {}
	virtual void WASABICALL CloudUploadCallback_OnDownloadFinished(nx_uri_t filename, ifc_clouddb *db_connection, const JSON::Value *value) {}
	virtual void WASABICALL CloudUploadCallback_OnError(nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field) {}
	virtual int WASABICALL CloudUploadCallback_IsKilled() { return 0; }
};

struct DeviceInfoStruct
{
	DeviceInfoStruct()
	{
		name = 0;
		type = 0;
		platform = 0;
	}

	~DeviceInfoStruct()
	{
		NXStringRelease(name);
		NXStringRelease(type);
		NXStringRelease(platform);
	}

	nx_string_t name, type, platform;
};

struct RenameStruct
{
	RenameStruct()
	{
		name = 0;
		old_name = 0;
		device = 0;
	}

	~RenameStruct()
	{
		NXStringRelease(name);
		NXStringRelease(old_name);
		NXStringRelease(device);
	}

	nx_string_t name;
	nx_string_t old_name;
	nx_string_t device;
};

struct RemoveStruct
{
	RemoveStruct()
	{
		device_id = -1;
		device_token = 0;
	}

	~RemoveStruct()
	{
		NXStringRelease(device_token);
	}

	int device_id;
	nx_string_t device_token;
};

struct UserProfileStruct
{
	UserProfileStruct()
	{
		full_name = 0;
		friendly_name = 0;
	}

	~UserProfileStruct()
	{
		NXStringRelease(full_name);
		NXStringRelease(friendly_name);
	}

	nx_string_t full_name, friendly_name;
};

struct PlaylistStruct
{
	PlaylistStruct()
	{
		playlist_id = 0;
		uuid = 0;
		name = 0;
		duration = 0;
		entries = 0;
		lastupdated = 0;
		priorupdate = 0;
	}

	~PlaylistStruct()
	{
		NXStringRelease(uuid);
		NXStringRelease(name);
	}

	nx_string_t uuid, name;
	int64_t playlist_id, entries, lastupdated, priorupdate;
	double duration;
};

struct PlaylistEntry
{
	PlaylistEntry()
	{
		location=0;
		title=0;
		duration=0;
		media_id=0;
		metahash=0;
		mediahash=0;
	}

	~PlaylistEntry()
	{
		NXStringRelease(location);
		NXStringRelease(title);
		NXStringRelease(metahash);
		NXStringRelease(mediahash);
	}

	nx_string_t location, title, metahash, mediahash;
	int duration;
	int64_t media_id;
};

#define ALL_SOURCES_CLIENT "all_sources"
#define HSS_CLIENT "hss"
#define DROPBOX_CLIENT "dropbox"
#define OLE_WEB_CLIENT "ole-web-client"

class cb_cloudevents : public nu::PtrDequeNode, public Wasabi2::Dispatchable
{
	protected:
	cb_cloudevents() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~cb_cloudevents() {}
public:
	void OnDeviceAdded(ifc_cloudclient *client, nx_string_t device_token, int device_id, DeviceInfoStruct *deviceInfo) { CloudEvents_OnDeviceAdded(client, device_token, device_id, deviceInfo); }
	void OnDeviceChanged(ifc_cloudclient *client, nx_string_t device_token, int device_id, nx_string_t device_name, DeviceInfoStruct *deviceInfo) { CloudEvents_OnDeviceChanged(client, device_token, device_id, device_name, deviceInfo); }
	void OnDeviceRemoved(ifc_cloudclient *client, int device_id) { CloudEvents_OnDeviceRemoved(client, device_id); }

	void OnRevision(ifc_cloudclient *client, int64_t revision, int from_reset) { CloudEvents_OnRevision(client, revision, from_reset); }
	void OnID(ifc_cloudclient *client, nx_uri_t filename, int internal_id) { CloudEvents_OnID(client, filename, internal_id); }
	void OnFirstPull(ifc_cloudclient *client, bool forced) { CloudEvents_OnFirstPull(client, forced); }
	void OnError(ifc_cloudclient *client, nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field) { CloudEvents_OnError(client, action, code, message, field); }
	void OnAction(ifc_cloudclient *client, nx_string_t action, nx_string_t message) { CloudEvents_OnAction(client, action, message); }

	void OnUnauthorized(ifc_cloudclient *client) { CloudEvents_OnUnauthorized(client); }
	void OnUserProfile(ifc_cloudclient *client, UserProfileStruct *userProfile) { CloudEvents_OnUserProfile(client, userProfile); }

	void OnUploadStart(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message) { CloudEvents_OnUploadStart(client, filepath, message); }
	void OnUploadDone(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message, int code) { CloudEvents_OnUploadDone(client, filepath, message, code); }

	void OnPlaylistAddUpdate(ifc_cloudclient *client, ifc_clouddb *db_connection, int mode, PlaylistStruct *playlist) { CloudEvents_OnPlaylistAddUpdate(client, db_connection, mode, playlist); }
	void OnPlaylistRemove(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid) { CloudEvents_OnPlaylistRemove(client, db_connection, uuid); }
	void OnPlaylistUpload(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid, int entry, PlaylistEntry *item) { CloudEvents_OnPlaylistUpload(client, db_connection, uuid, entry, item); }
	void OnPlaylistsDone(ifc_cloudclient *client, ifc_clouddb *db_connection) { CloudEvents_OnPlaylistsDone(client, db_connection); }

	virtual int WASABICALL CloudEvents_GetRunDeviceEnum() { return 0; }
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual void WASABICALL CloudEvents_OnDeviceAdded(ifc_cloudclient *client, nx_string_t device_token, int device_id, DeviceInfoStruct *deviceInfo) {}
	virtual void WASABICALL CloudEvents_OnDeviceChanged(ifc_cloudclient *client, nx_string_t device_token, int device_id, nx_string_t device_name, DeviceInfoStruct *deviceInfo) {}
	virtual void WASABICALL CloudEvents_OnDeviceRemoved(ifc_cloudclient *client, int device_id) {}
	virtual void WASABICALL CloudEvents_OnRevision(ifc_cloudclient *client, int64_t revision, int from_reset) {}
	virtual void WASABICALL CloudEvents_OnID(ifc_cloudclient *client, nx_uri_t filename, int internal_id) {}
	virtual void WASABICALL CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced) {}
	virtual void WASABICALL CloudEvents_OnError(ifc_cloudclient *client, nx_string_t action, nx_string_t code, nx_string_t message, nx_string_t field) {}
	virtual void WASABICALL CloudEvents_OnAction(ifc_cloudclient *client, nx_string_t action, nx_string_t message) {}
	virtual void WASABICALL CloudEvents_OnUnauthorized(ifc_cloudclient *client) {}
	virtual void WASABICALL CloudEvents_OnUserProfile(ifc_cloudclient *client, UserProfileStruct *userProfile) {}

	virtual void WASABICALL CloudEvents_OnUploadStart(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message) {}
	virtual void WASABICALL CloudEvents_OnUploadDone(ifc_cloudclient *client, nx_uri_t filepath, nx_string_t message, int code) {}

	virtual void WASABICALL CloudEvents_OnPlaylistAddUpdate(ifc_cloudclient *client, ifc_clouddb *db_connection, int mode, PlaylistStruct *playlist) {}
	virtual void WASABICALL CloudEvents_OnPlaylistRemove(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid) {}
	virtual void WASABICALL CloudEvents_OnPlaylistUpload(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid, int entry, PlaylistEntry* item) {}
	virtual void WASABICALL CloudEvents_OnPlaylistsDone(ifc_cloudclient *client, ifc_clouddb *db_connection) {}
};