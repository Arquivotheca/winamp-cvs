#include "CloudThread.h"
#include "nswasabi/ReferenceCounted.h"

void CloudThread::Parse_UserProfile(const JSON::Value *root)
{
	const JSON::Value *json_user_profile;
	if (root->FindNextKey(0, "user-profile", &json_user_profile) == NErr_Success)
	{
		const JSON::Value *json_name;
		if (json_user_profile->FindNextKey(0, "name", &json_name) == NErr_Success)
		{
			ReferenceCountedNXString friendly_name, full_name;

			const JSON::Value *value;
			if (json_name->FindNextKey(0, "friendly", &value) == NErr_Success)
				value->GetString(&friendly_name);

			if (json_name->FindNextKey(0, "full", &value) == NErr_Success)
				value->GetString(&full_name);

			UserProfileStruct *userProfile = new (std::nothrow) UserProfileStruct;
			userProfile->full_name = full_name;
			userProfile->friendly_name = friendly_name;
			for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
			{
				itr->OnUserProfile(this, userProfile);
			}
			delete userProfile;
		}
	}
}