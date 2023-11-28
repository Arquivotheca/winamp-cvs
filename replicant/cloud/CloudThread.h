#pragma once
#include "foundation/error.h"
#include "CloudSocket.h"
#include "nu/ThreadLoop.h"
#include "TransactionQueue.h"
#include "nu/PtrDeque.h"
#include "ifc_cloudclient.h"
#include "nx/nxthread.h"
#include "JSON-Tree.h"
#include "ssdp/cb_ssdp.h"

class Cloud_DBConnection;
class ifc_clouddb;
struct UploadStruct
{
	UploadStruct()
	{
		filename=0;
		destination_device=0;
		callback=0;
		internal_id=0;
	}

	~UploadStruct()
	{
		NXURIRelease(filename);
		NXStringRelease(destination_device);
		if (callback)
			callback->Release();
	}

	nx_uri_t filename;
	nx_string_t destination_device;
	int internal_id;
	cb_cloud_upload *callback;
};

struct LANDeviceStruct :  public nu::PtrDequeNode
{
	LANDeviceStruct()
	{
		location=0;
		usn=0;
		device_token=0;
	}

	~LANDeviceStruct()
	{
		NXURIRelease(location);
		NXStringRelease(usn);
		NXStringRelease(device_token);
	}

	nx_string_t usn;
	nx_uri_t location;
	nx_string_t device_token;
};

class CloudThread : public ifc_cloudclient, private cb_ssdp
{
public:
	CloudThread();
	~CloudThread();
	ns_error_t Initialize(nx_string_t device_id, int dev_mode);
	static ns_error_t CreatePathForDatabase(nx_uri_t *filepath, nx_string_t device_id, int dev_mode);

	ns_error_t WASABICALL CloudClient_MetadataAnnounce1(ifc_clouddb *_db_connection, int internal_id);
	ns_error_t WASABICALL CloudClient_Reset();
	ns_error_t WASABICALL CloudClient_Flush(bool restart);

	ns_error_t WASABICALL CloudClient_Upload(nx_uri_t filename, nx_string_t destination_device, int internal_id, cb_cloud_upload *callback);
	ns_error_t WASABICALL CloudClient_Download(nx_uri_t destination, int internal_id, cb_cloud_upload *callback);

	ns_error_t WASABICALL CloudClient_DownloadPlaylist(nx_uri_t destination, nx_string_t uuid, int internal_id, cb_cloud_upload *callback);

	ns_error_t WASABICALL CloudClient_DeviceUpdate();
	ns_error_t WASABICALL CloudClient_DeviceRemove(RemoveStruct *remove);
	ns_error_t WASABICALL CloudClient_DeviceRename(RenameStruct *rename);
	ns_error_t WASABICALL CloudClient_RegisterCallback(cb_cloudevents *callback);
	// call this to create a database connection on the current thread.
	ns_error_t WASABICALL CloudClient_CreateDatabaseConnection(ifc_clouddb **out_connection);

private:
	/* cb_ssdp implementation */
	void WASABICALL SSDPCallback_OnServiceConnected(nx_uri_t location, nx_string_t type, nx_string_t usn);
	void WASABICALL SSDPCallback_OnServiceDisconnected(nx_string_t usn);
private:
	static nx_thread_return_t NXTHREADCALL RunFunc(nx_thread_parameter_t parameter) { return ((CloudThread *)parameter)->Run(); }
	static void APC_Queue(void *_cloud_thread, void *transaction, double) { ((CloudThread *)_cloud_thread)->Internal_Queue((Transaction *)transaction); }

	static void APC_Flush(void *_cloud_thread, void *, double) { /* Purposefully do nothing */ }
	static void APC_GetRevision(void *_cloud_thread, void *, double) { ((CloudThread *)_cloud_thread)->Internal_GetRevision(); }

	static void APC_Reset(void *_cloud_thread, void *, double) { ((CloudThread *)_cloud_thread)->Internal_Reset(); }
	static void APC_Upload(void *_cloud_thread, void *upload, double) { ((CloudThread *)_cloud_thread)->Internal_Upload((UploadStruct *)upload); }
	static void APC_Download(void *_cloud_thread, void *download, double) { ((CloudThread *)_cloud_thread)->Internal_Download((UploadStruct *)download); }
	static void APC_DownloadPlaylist(void *_cloud_thread, void *download, double) { ((CloudThread *)_cloud_thread)->Internal_DownloadPlaylist((UploadStruct *)download); }
	static void APC_DeviceUpdate(void *_cloud_thread, void * remove, double) { ((CloudThread *)_cloud_thread)->Internal_DeviceUpdate(); }
	static void APC_DeviceRemove(void *_cloud_thread, void * remove, double) { ((CloudThread *)_cloud_thread)->Internal_DeviceRemove((RemoveStruct *)remove); }
	static void APC_DeviceRename(void *_cloud_thread, void * rename, double) { ((CloudThread *)_cloud_thread)->Internal_DeviceRename((RenameStruct *)rename); }
	static void APC_RegisterCallback(void *_cloud_thread, void *callback, double) { ((CloudThread *)_cloud_thread)->Internal_RegisterCallback((cb_cloudevents *)callback); }
	static void APC_OnServiceConnected(void *_cloud_thread, void *data, double) { ((CloudThread *)_cloud_thread)->Internal_OnServiceConnected((LANDeviceStruct *)data); }
	static void APC_OnServiceDisconnected(void *_cloud_thread, void *usn, double) { ((CloudThread *)_cloud_thread)->Internal_OnServiceDisconnected((nx_string_t)usn); }
	void Internal_Queue(Transaction *transaction);
	void Internal_Initialize();
	ns_error_t Internal_Pull();
	void Internal_GetRevision();
	void Internal_Reset();
	void Internal_Upload(UploadStruct *upload);
	void Internal_Download(UploadStruct *upload);
	void Internal_DownloadPlaylist(UploadStruct *upload);
	ns_error_t Internal_DevicesList();
	void Internal_Announce();
	void Internal_Remove();
	void Internal_RegisterCallback(cb_cloudevents *callback);
	void Internal_DeviceUpdate();
	void Internal_DeviceRemove(RemoveStruct *remove);
	void Internal_DeviceRename(RenameStruct *rename);
	void Internal_Update();
	void Internal_OnServiceConnected(LANDeviceStruct *data);
	void Internal_OnServiceDisconnected(nx_string_t usn);
	ns_error_t Internal_UserProfile();
	void Internal_Playlists();
	void Internal_Playlists_Add(nx_string_t uuid, int64_t playlist_id);
	void Internal_Playlists_Remove(nx_string_t uuid, int64_t playlist_id);
	ns_error_t Internal_Playlists_Snapshot();

	int PostJSON(yajl_gen json, nsjson_tree_t *output=0, const char *subdir=0);
	nx_thread_return_t Run();
	void ProcessQueue();
	void Parse_DevicesList(const JSON::Value *root);

	void Parse_Playlist_Snapshot(const JSON::Value *root);
	void Parse_Playlist_Add_Update(const JSON::Value *root);
	void Parse_Playlist_Remove(const JSON::Value *root);

	void Parse_UserProfile(const JSON::Value *root);
	int Parse_Pull(const JSON::Value *root);
	void Parse_Action(const JSON::Value *cmd, const JSON::Value_Map *action);
	void Parse_Action_Add(const JSON::Value *cmd, const JSON::Value *fields);
	void Parse_Action_Delete(const JSON::Value *cmd, const JSON::Value *fields);
	void Parse_Action_Remove(const JSON::Value *cmd, const JSON::Value *fields);
	void Parse_Action_Played(const JSON::Value *cmd, const JSON::Value *fields);
	void Parse_Action_Update(const JSON::Value *cmd, const JSON::Value *fields);
	int Parse_Error(const JSON::Value *value, const JSON::Value *code, const char *subdir);
	int Parse_Upload_Error(UploadStruct *upload, const JSON::Value *value, const char *subdir);

	int UploadLAN(UploadStruct *upload);
	int UploadServer(UploadStruct *upload);
	int UploadPlaylist(nx_string_t uuid, int64_t playlist_id, int set);

	int DownloadServer(UploadStruct *upload);
	int DownloadPlaylist(UploadStruct *upload);

	Cloud_DBConnection *db_connection;
	CloudSocket cloud_socket;
	ThreadLoop thread_loop;
	nu::PtrDeque<Transaction> transaction_queue;
	nx_string_t device_id;
	bool pull_required;
	bool first_pull;
	int device_refresh;
	bool playlist_refresh;
	nx_thread_t cloud_thread;
	Attributes attributes;
	typedef nu::PtrDeque<cb_cloudevents> CallbackList;
	CallbackList callbacks;
	size_t queue_size;
	int killswitch;
	int dev_mode;
	typedef nu::PtrDeque<LANDeviceStruct> LANDevices;
	LANDevices lan_devices;
};