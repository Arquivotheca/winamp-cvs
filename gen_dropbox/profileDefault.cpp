#include "./main.h"
#include "./dropWindow.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./simpleView.h"
#include "./detailsView.h"
#include "./filterPolicy.h"
#include "./itemTypeInterface.h"
#include "./meterbar.h"
#include "./profile.h"
#include "./profileManager.h"

#include "./configIniSection.h"


#define CONFIG_SECTION(__sectionGuid) { NULL, ConfigItemTypeSection, ((LPCTSTR)&(__sectionGuid)) }
#define CONFIGITEM_STR(__key, _value) { (__key), ConfigItemTypeString, ((LPCTSTR)(_value)) }
#define CONFIGITEM_INT(__key, _value) { (__key), ConfigItemTypeInteger, MAKEINTRESOURCE(_value) }
#define CONFIGFILTER_RULE(__itemType, __rule) { MAKEINTRESOURCEA(IItemType::##__itemType), ConfigItemTypeFilterPolicy, MAKEINTRESOURCE(FilterPolicy::##__rule) }

static const CONFIGITEMTEMPLATE audioProfile[] =
{
	CONFIG_SECTION(windowSettingsGuid), // window
		CONFIGITEM_STR(CFG_ACTIVEVIEW, SIMPLEVIEW_NAME),

	CONFIG_SECTION(simpleViewSettingsGuid),
		CONFIGITEM_INT(CFG_SHOWINDEX, TRUE), 
		CONFIGITEM_INT(CFG_SHOWTYPEICON, TRUE), 
		CONFIGITEM_STR(CFG_RCOLUMNSOURCE, GET_REGISTERED_COLUMN_NAME(COLUMN_TRACKLENGTH)), 

	CONFIG_SECTION(meterbarSettingsGuid),
	CONFIGITEM_STR(CFG_METERUNIT, Meterbar_GetUnitName(Meterbar::FlagUnitLength)), 
	
	CONFIGFILTER_RULE(itemTypeMissingFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeUnknown, entryRuleIgnore),
	CONFIGFILTER_RULE(itemTypeAudioFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeVideoFile, entryRuleIgnore),
	CONFIGFILTER_RULE(itemTypePlaylistFile, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeFolder, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeLinkFile, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeHttpStream, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeAudioCdTrack, entryRuleAdd),
};

static const CONFIGITEMTEMPLATE dataProfile[] =
{
	CONFIG_SECTION(windowSettingsGuid), // window
		CONFIGITEM_STR(CFG_ACTIVEVIEW, SIMPLEVIEW_NAME),

	CONFIG_SECTION(simpleViewSettingsGuid),
		CONFIGITEM_INT(CFG_SHOWINDEX, TRUE), 
		CONFIGITEM_INT(CFG_SHOWTYPEICON, TRUE), 
		CONFIGITEM_STR(CFG_RCOLUMNSOURCE, GET_REGISTERED_COLUMN_NAME(COLUMN_TRACKLENGTH)), 

	CONFIG_SECTION(meterbarSettingsGuid),
	CONFIGITEM_STR(CFG_METERUNIT, Meterbar_GetUnitName(Meterbar::FlagUnitSize)), 
	
	CONFIGFILTER_RULE(itemTypeMissingFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeUnknown, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeAudioFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeVideoFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypePlaylistFile, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeFolder, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeLinkFile, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeHttpStream, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeAudioCdTrack, entryRuleAdd),
	
};

static const CONFIGITEMTEMPLATE debugProfile[] =
{
	CONFIG_SECTION(windowSettingsGuid), // window
		CONFIGITEM_STR(CFG_ACTIVEVIEW, DETAILSVIEW_NAME),

	CONFIGFILTER_RULE(itemTypeMissingFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeUnknown, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeAudioFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypeVideoFile, entryRuleAdd),
	CONFIGFILTER_RULE(itemTypePlaylistFile, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeFolder, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeLinkFile, entryRuleEnumerate),
	CONFIGFILTER_RULE(itemTypeHttpStream, entryRuleAsk),
	CONFIGFILTER_RULE(itemTypeAudioCdTrack, entryRuleAdd),
	
};
	
static const PROFILETEMPLATE profileTemplates[] =
{
	{ { 0x297dea7e, 0x6867, 0x447e, { 0x95, 0xa9, 0x6a, 0x76, 0xda, 0x84, 0x27, 0xf2 } },
		MAKEINTRESOURCE(IDS_PROFILE_AUDIO), MAKEINTRESOURCE(IDS_PROFILE_AUDIO_DESCRIPTION), 
		ARRAYSIZE(audioProfile), audioProfile},

	//{ { 0x96174879, 0x7f60, 0x4278, { 0xb6, 0x60, 0xc1, 0x46, 0x76, 0x0, 0x79, 0xa4 } },
	//	MAKEINTRESOURCE(IDS_PROFILE_DATA), MAKEINTRESOURCE(IDS_PROFILE_DATA_DESCRIPTION),
	//	ARRAYSIZE(dataProfile), dataProfile},
	
	//{ { 0xed370039, 0x6ebe, 0x4b3b, { 0xb6, 0x22, 0xc0, 0xdc, 0xb4, 0xf3, 0xd, 0xf } },
	//	TEXT("Debug Mode"), TEXT("This profile is tuned to display more information and can be useful for chasing bugs and discovering new issues."), 
	//	ARRAYSIZE(debugProfile), debugProfile},

};


INT ProfileManager::RegisterDefault()
{
	INT registeredCount = 0;
	Profile *profile;
	for (INT i =0; i < ARRAYSIZE(profileTemplates); i++)
	{
		profile = Profile::CreateTemplate(&profileTemplates[i]);
		if (NULL != profile)
		{
			profile->Release();
			registeredCount++;
		}
	}
	return registeredCount;
}