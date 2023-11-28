#include "CloudThread.h"
#include "main.h"
#include "api.h"
#include "JSON-Tree.h"
#include "CloudDB.h"
#include "Attributes.h"
#include "nswasabi/ReferenceCounted.h"
#include "CloudAPI.h"
#include "JSONMetadata.h"

extern CloudAPI cloudApi;

void CloudThread::Parse_Action_Update(const JSON::Value *cmd, const JSON::Value *fields)
{
	// build and ifc_metadata interface and update via Media_Update
	const JSON::Value *value;

	int64_t cloud_id;
	int internal_id;
	if (fields->FindNextKey(0, "id", &value, 0) == NErr_Success && value->GetInteger(&cloud_id) == NErr_Success)
	{
		int is_ignored=0;
		if (db_connection->IDMap_Find(cloud_id, &internal_id) == NErr_Success && is_ignored==0) // TODO: benski> think thru all is_ignored cases here, wrt to doing a from-revision=0 update
		{
			/* we're not going to reference count this.  
			If we ever change Media_Update to keep the metadata object past the lifetime of the function, we'll need to "new" it instead */
			JSONMetadata update_metadata(cmd, fields);
			int dirty_flags = ifc_clouddb::DIRTY_NONE;
			ReferenceCountedNXString device_token;
			if (update_metadata.GetField(MetadataKey_CloudDevice, 0, &device_token) == NErr_Success && !NXStringCompare(device_token, attributes.device_token, nx_compare_default))
				dirty_flags = ifc_clouddb::DIRTY_REMOTE;
			db_connection->Media_Update(internal_id, &update_metadata, dirty_flags);
		}
	}
}