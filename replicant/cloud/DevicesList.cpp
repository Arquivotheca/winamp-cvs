#include "main.h"
#include "CloudThread.h"
#include "JSON-Tree.h"
#include "CloudDB.h"
#include "Attributes.h"
#include "nu/vector.h"
#include "nswasabi/ReferenceCounted.h"

void CloudThread::Parse_DevicesList(const JSON::Value *root)
{
	const JSON::Value *devices_array = 0;
	if (root->FindNextKey(0, "devices", &devices_array) == NErr_Success)
	{
		const JSON::Value *device = 0;
		size_t i = 0;
		Vector<int, 32, 2> ids;

		while (devices_array->EnumerateValues(i++, &device) == NErr_Success)
		{
			DeviceInfoStruct *device_info = new (std::nothrow) DeviceInfoStruct; // TODO I think we don't need to "new" this, just put it on the stack and pass &device_info
			ReferenceCountedNXString token, device_name;
			const JSON::Value *value;
			device->FindNextKey(0, "dev", &value);
			value->GetString(&token);
			device->FindNextKey(0, "name", &value);
			value->GetString(&device_name);


			const JSON::Value *status;
			device->FindNextKey(0, "status", &status);

			int64_t on = 0, last_seen = 0, transient=1;
			double availability=0.0;
			if (status->FindNextKey(0, "on", &value) == NErr_Success)
				value->GetInteger(&on);
			if (status->FindNextKey(0, "last-seen", &value) == NErr_Success)
				value->GetInteger(&last_seen);
			if (status->FindNextKey(0, "transient", &value) == NErr_Success)
				value->GetInteger(&transient);
			if (status->FindNextKey(0, "availability", &value) == NErr_Success)
				value->GetDouble(&availability);


			const JSON::Value *description;
			device->FindNextKey(0, "descr", &description);

			if (device_info)
			{
				const JSON::Value *hardware;
				description->FindNextKey(0, "hardware", &hardware);
				hardware->FindNextKey(0, "type", &value);
				value->GetString(&device_info->type);
				hardware->FindNextKey(0, "platform", &value);
				value->GetString(&device_info->platform);
			}


			const JSON::Value *capabilities;
			device->FindNextKey(0, "capab", &capabilities);

			const JSON::Value *cache;
			capabilities->FindNextKey(0, "cache", &cache);

			int64_t total = 0, used = 0;
			const JSON::Value *total_size;
			cache->FindNextKey(0, "size", &total_size);
			total_size->GetInteger(&total);
			cache->FindNextKey(0, "used", &total_size);
			total_size->GetInteger(&used);

			
			// check for it being our device and update the name
			// as this will preserve a custom name on db removal
			if (!NXStringKeywordCompare(attributes.device_token, token))
				db_connection->CloudDB_Info_SetDeviceName(device_name);

			int device_id = 0;
			ns_error_t ret = db_connection->Devices_Add(token, device_name, device_info, &device_id);
			if (ret == NErr_Success || ret == NErr_NoAction)
			{
						// benski> can we do this outside of the Transaction somehow?
				for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
				{
					if (ret == NErr_Success)
					{
						itr->OnDeviceAdded(this, token, device_id, device_info);
					}
					else
					{
						itr->OnDeviceChanged(this, token, device_id, device_name, device_info);
					}
				}
				ids.push_back(device_id);
				db_connection->Devices_StoreCapacity(device_id, total, used);
				db_connection->Devices_SetLastSeen(device_id, last_seen, (int)on);
				db_connection->Devices_SetAvailability(device_id, transient, availability);
			}
			delete device_info;
		}

		// this checks through the devices which have been reported and if there are any
		// not found then we treat them as removed and so remove from the db and showing
		int *device_ids = 0;
		size_t num_devices = 0;
		if (db_connection->Devices_GetDeviceIDs(&device_ids, &num_devices) == NErr_Success)
		{
			for (i = 0; i < num_devices; i++)
			{
				size_t d = 0;
				for (; d < ids.size(); d++)
				{
					if (device_ids[i] == ids[d])
					{
						break;
					}
				}
				if (d == ids.size())
				{
					for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
					{
						// benski> can we do this outside of the Transaction somehow?
						itr->OnDeviceRemoved(this, device_ids[i]);
					}
					db_connection->Devices_Remove(device_ids[i]);
				}
			}
		}
		if (device_ids) free(device_ids);
	}
}