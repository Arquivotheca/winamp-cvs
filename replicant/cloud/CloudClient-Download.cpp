#include "api.h"
#include "CloudThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include "nswasabi/AutoCharNX.h"
#include "CloudDB.h"

extern CloudAPI cloud_api;

int CloudThread::DownloadServer(UploadStruct *upload)
{
	ReferenceCountedNXString username, cloud_url;
	REPLICANT_API_CLOUD->Cloud_GetAPIURL(&cloud_url, /*http=*/NErr_True);
	if (REPLICANT_API_CLOUD->GetCredentials(&username, 0, 0) != NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unauthorized);
		return NErr_Success; // TODO?
	}

	int64_t cloud_id = 0;
	if (db_connection->IDMap_Get(upload->internal_id, &cloud_id) != NErr_Success || !cloud_id)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unknown);

		return NErr_Success; // TODO?
	}

	ReferenceCountedNXString mediahash;
	if (db_connection->IDMap_GetMediaHash(upload->internal_id, &mediahash) != NErr_Success)
	{
		if (upload->callback)
			upload->callback->OnFinished(NErr_Unknown);

		return NErr_Success; // TODO?
	}

	char url[1024];
	sprintf(url, "%sdemostream/%s/%llu/%s", AutoCharPrintfUTF8(cloud_url), AutoCharPrintfUTF8(username), cloud_id, AutoCharPrintfUTF8(mediahash));

	cloud_socket.DownloadFile(url, upload->filename, 0, 0, upload->callback, 0);

	if (upload->callback)
		upload->callback->OnFinished(NErr_Success);

	return NErr_Success;
}

void CloudThread::Internal_Download(UploadStruct *upload)
{
	DownloadServer(upload);
	delete upload;
}

ns_error_t CloudThread::CloudClient_Download(nx_uri_t destination, int internal_id, cb_cloud_upload *callback)
{
		UploadStruct *upload = new (std::nothrow) UploadStruct;
	if (!upload)
		return NErr_OutOfMemory;

	threadloop_node_t *apc = thread_loop.GetAPC();
	if (!apc)
	{
		delete upload;
		return NErr_OutOfMemory;
	}

	upload->filename = NXURIRetain(destination);
	upload->destination_device = 0;//NXStringRetain(destination_device);
	upload->internal_id = internal_id;
	upload->callback = callback;
	if (callback)
		callback->Retain();

	apc->func = APC_Download;
	apc->param1 = this;
	apc->param2 = upload;
	thread_loop.Schedule(apc);
	return NErr_Success;
}