#pragma once
#include <jni.h>
#include <cloud/ifc_clouddb.h>
#include <cloud/ifc_cloudclient.h>
#include <cloud/cb_cloudevents.h>
#include <cloud/Attributes.h>
#include "api.h"

#define USE_PRODUCTION_API 0
#define USE_DEVELOPMENT_API 1

class JNICloudManager : public cb_cloudevents
{
public:
	JNICloudManager(JNIEnv *env, jobject obj);
	int InitializeDB(nx_string_t device_token);
	void CalculateMediaHashes();
	ifc_cloudclient *cloud_client;
	ifc_clouddb *db_connection;

	static JNINativeMethod jni_methods[];
	static size_t jni_methods_count;
	static const char *jni_classname;
private:
	jobject java_cloud_manager;
	Attributes attributes;
	
	/* cloud event callbacks */
	void WASABICALL CloudEvents_OnRevision(ifc_cloudclient *client, int64_t revision, int from_reset);
	void WASABICALL CloudEvents_OnUnauthorized(ifc_cloudclient *client);
	void WASABICALL CloudEvents_OnAction(ifc_cloudclient *client, nx_string_t action, nx_string_t message);
	void WASABICALL CloudEvents_OnFirstPull(ifc_cloudclient *client, bool forced);
	void WASABICALL CloudEvents_OnPlaylistUpload(ifc_cloudclient *client, ifc_clouddb *db_connection, nx_string_t uuid, int entry, PlaylistEntry* item);
};
