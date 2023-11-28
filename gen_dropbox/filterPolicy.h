#ifndef NULLOSFT_WINAMP_DROPBOX_PLUGIN_FILTERPOLICY_HEADER
#define NULLOSFT_WINAMP_DROPBOX_PLUGIN_FILTERPOLICY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

interface IItemType;
class Profile;

class FilterPolicy
{

public:
	typedef enum
	{
		entryRuleError = ((BYTE)-1),
		entryRuleAsk = 0,
		entryRuleAdd,
		entryRuleIgnore,
		entryRuleEnumerate,
		entryRuleLast,
	} FilterRules;

public:
	static FilterPolicy* Load(Profile *profile);
	static FilterPolicy* Create();

protected:
	FilterPolicy();
	~FilterPolicy();

public:
	ULONG AddRef(void);
	ULONG Release(void);

	BOOL AddEntry(IItemType *itemType, UINT typeRule);
	BOOL ReadRegisteredTypes();
	UINT GetRule(BYTE typeId);
	BOOL SetRule(BYTE typeId, UINT typeRule);

	HRESULT Save(Profile *profile);
	static LPCTSTR GetRuleName(BYTE ruleId);
	static HRESULT GetRuleDisplayName(BYTE ruleId, LPTSTR pszBuffer, INT cchBufferMax);

	FilterPolicy *Clone();
	
private:
	typedef struct __Entry
	{
		BYTE typeId;
		BYTE rule;
	} Entry;

private:
	ULONG	ref;
	Entry	*list;
	INT		listSize;
	INT		listCount;
	INT		cacheIndex;
};


#endif //NULLOSFT_WINAMP_DROPBOX_PLUGIN_FILTERPOLICY_HEADER