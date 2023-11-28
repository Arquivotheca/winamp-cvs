#pragma once
#include "foundation/dispatch.h"
#include "nx/nxuri.h"
#include "foundation/error.h"
#include "cb_cloudevents.h"

class ifc_clouddb;

class ifc_cloudclient : public Wasabi2::Dispatchable
{
protected:
	ifc_cloudclient() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_cloudclient() {}
public:
	ns_error_t MetadataAnnounce1(ifc_clouddb *_db_connection, int internal_id) { return CloudClient_MetadataAnnounce1(_db_connection, internal_id); }
	ns_error_t Reset() { return CloudClient_Reset(); }
	ns_error_t Flush(bool restart=false) { return CloudClient_Flush(restart); }
	
	// call this to create a database connection on the current thread.
	ns_error_t CreateDatabaseConnection(ifc_clouddb **out_connection) { return CloudClient_CreateDatabaseConnection(out_connection); }

	ns_error_t Upload(nx_uri_t filename, nx_string_t destination_device, int internal_id, cb_cloud_upload *callback)
		{ return CloudClient_Upload(filename, destination_device, internal_id, callback); }

	ns_error_t Download(nx_uri_t filename, int internal_id, cb_cloud_upload *callback)
		{ return CloudClient_Download(filename, internal_id, callback); }

	ns_error_t DownloadPlaylist(nx_uri_t filename, nx_string_t uuid, int internal_id, cb_cloud_upload *callback)
		{ return CloudClient_DownloadPlaylist(filename, uuid, internal_id, callback); }

	ns_error_t DeviceUpdate() { return CloudClient_DeviceUpdate(); }
	ns_error_t DeviceRemove(RemoveStruct *remove) { return CloudClient_DeviceRemove(remove); }
	ns_error_t DeviceRename(RenameStruct *rename) { return CloudClient_DeviceRename(rename); }

	ns_error_t RegisterCallback(cb_cloudevents *callback) { return CloudClient_RegisterCallback(callback); }
private:
	
	virtual ns_error_t WASABICALL CloudClient_MetadataAnnounce1(ifc_clouddb *_db_connection, int internal_id)=0;
	virtual ns_error_t WASABICALL CloudClient_Reset()=0;
	virtual ns_error_t WASABICALL CloudClient_Flush(bool restart)=0;
	
	virtual ns_error_t WASABICALL CloudClient_CreateDatabaseConnection(ifc_clouddb **out_connection)=0;
	virtual ns_error_t WASABICALL CloudClient_Upload(nx_uri_t filename, nx_string_t destination_device, int internal_id, cb_cloud_upload *callback)=0;
	virtual ns_error_t WASABICALL CloudClient_Download(nx_uri_t destination, int internal_id, cb_cloud_upload *callback)=0;
	virtual ns_error_t WASABICALL CloudClient_DownloadPlaylist(nx_uri_t destination, nx_string_t uuid, int internal_id, cb_cloud_upload *callback)=0;
	virtual ns_error_t WASABICALL CloudClient_DeviceUpdate()=0;
	virtual ns_error_t WASABICALL CloudClient_DeviceRemove(RemoveStruct *remove)=0;
	virtual ns_error_t WASABICALL CloudClient_DeviceRename(RenameStruct *rename)=0;
	virtual ns_error_t WASABICALL CloudClient_RegisterCallback(cb_cloudevents *callback)=0;
	
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
	
};