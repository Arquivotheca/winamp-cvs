#include "main.h"
#include "./plugin.h"
#include "./filterPolicy.h"
#include "./formatData.h"
#include "./wasabiApi.h"
#include "./profile.h"
#include "./resource.h"
#include <strsafe.h>


static const LPCTSTR szRuleNames[] =
{
	TEXT("ask"),
	TEXT("add"),
	TEXT("ignore"),
	TEXT("enumerate"),
};

static BOOL CALLBACK InsertItems_EnumProc(IItemType *item, ULONG_PTR param)
{
	FilterPolicy *pfp = (FilterPolicy*)param;
	if (NULL == pfp)
		return FALSE;
	
	pfp->AddEntry(item, FilterPolicy::entryRuleAsk);

	return TRUE;
}

FilterPolicy::FilterPolicy() :
	ref(1), list(NULL), listSize(0), listCount(0), cacheIndex(0)
{
	
}

FilterPolicy::~FilterPolicy()
{
	if (NULL != list)
		free(list);
}

ULONG FilterPolicy::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

ULONG FilterPolicy::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

typedef struct __PARSERLINEDATA
{
	BYTE		id;
	BYTE		rule;
} PARSERLINEDATA;

static UINT CALLBACK FilterPolicy_LineParser(LPCTSTR pszKeyword, INT cchKeyword, LPVOID user)
{
	PARSERLINEDATA *lineData = (PARSERLINEDATA*)user;

	if (((BYTE)-1) == lineData->id)
	{
		
		if (NULL != PLUGIN_REGTYPES) 
		{
			IItemType *type = PLUGIN_REGTYPES->FindByName(pszKeyword, cchKeyword);
			if (NULL != type)
				lineData->id = type->GetId();
		}
		if (((BYTE)-1) == lineData->id)
			return KWPARSER_ABORT;
	}
	else
	{
		for (INT i = 0; i < ARRAYSIZE(szRuleNames); i++)
		{
			if (CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, szRuleNames[i], -1, pszKeyword, cchKeyword))
			{
				lineData->rule = ((BYTE)i);
				return KWPARSER_FOUND | KWPARSER_ABORT;
			}
		}
	}
	return KWPARSER_CONTINUE;
}



FilterPolicy* FilterPolicy::Load(Profile *profile)
{
	INT cchBufferMax = 16000; // really? ..this must be enough to read whole section 
	LPTSTR pszBuffer = (LPTSTR)malloc(sizeof(TCHAR) * cchBufferMax); 
	if (NULL == pszBuffer)
		return NULL;

	TCHAR szPath[MAX_PATH * 2];
	if (NULL == profile || FAILED(profile->GetFilePath(szPath, ARRAYSIZE(szPath))))
		return NULL;
	
	INT cchBuffer = GetPrivateProfileSection(TEXT("FilterPolicy"), pszBuffer, cchBufferMax, szPath);
	if (cchBuffer == cchBufferMax - 2)
	{ 
		// "Sweet zombie Jesus!" (Professor Hubert J. Farnsworth)
		free(pszBuffer);
		return NULL;
	}

	FilterPolicy *instance = new FilterPolicy();
	if (NULL == instance)
		return NULL;

	instance->ReadRegisteredTypes();
	if (0 != cchBuffer)
	{
		LPCTSTR line = pszBuffer;
		while (TEXT('\0') != *line)
		{
			INT cchLine = lstrlen(line);
			
			if (TEXT(';') != *line) 
			{
				PARSERLINEDATA lineData;
				lineData.id = ((BYTE)-1);
				lineData.rule = FilterPolicy::entryRuleError;
				
				if (0 != ParseKeywords(line, cchLine, TEXT("="), TRUE, FilterPolicy_LineParser, &lineData) &&
					((BYTE)-1) != lineData.id && 
					FilterPolicy::entryRuleError != lineData.rule)
				{
					instance->SetRule(lineData.id, lineData.rule);
				}
			}

			line += (cchLine + 1);
		}
		
	}
	free(pszBuffer);
	return instance;
}

FilterPolicy* FilterPolicy::Create()
{
	FilterPolicy *instance = new FilterPolicy();
	if (NULL != instance)
		instance->ReadRegisteredTypes();
	return instance;
}


BOOL FilterPolicy::AddEntry(IItemType *itemType, UINT typeRule)
{
	if (NULL == itemType) 
		return FALSE;

	if (listCount == (listSize - 1))
	{
		INT newSize = listSize + listSize /2;
		if (newSize < 8) newSize = 8;

		if (newSize <= listSize) newSize = listSize + 2;
		void *data = realloc(list, sizeof(Entry) * newSize);
		if (NULL == data)
			return FALSE;
		listSize = newSize;
		list = (Entry*)data;
	}
	
	list[listCount].rule = typeRule;
	list[listCount].typeId = itemType->GetId();

	listCount++;
	cacheIndex = 0;
	return TRUE;
}

BOOL FilterPolicy::ReadRegisteredTypes()
{
	if (NULL == PLUGIN_REGTYPES)
		return FALSE;
	
	INT typeListSize = PLUGIN_REGTYPES->Count();
	if ((listSize - listCount) < typeListSize)
	{
		INT newSize = listCount + typeListSize;
		void *data = realloc(list, sizeof(Entry) * newSize);
		if (NULL == data)
			return FALSE;

		listSize = newSize;
		list = (Entry*)data;
	}

	PLUGIN_REGTYPES->Enumerate(InsertItems_EnumProc, (ULONG_PTR)this);
	cacheIndex = 0;
	return TRUE;
}

UINT FilterPolicy::GetRule(BYTE typeId)
{
	cacheIndex = 0;
	if (typeId == list[cacheIndex].typeId)
		return list[cacheIndex].rule;

	for (int i =0; i < listCount; i++)
	{
		if (typeId == list[i].typeId)
		{
			cacheIndex = i;
			return list[i].rule;
		}
	}
	cacheIndex = 0;
	return FilterPolicy::entryRuleError;
}

BOOL FilterPolicy::SetRule(BYTE typeId, UINT typeRule)
{
	for (int i =0; i < listCount; i++)
	{
		if (typeId == list[i].typeId)
		{
			list[i].rule = typeRule;
			return TRUE;
		}
	}
	return FALSE;
}


LPCTSTR FilterPolicy::GetRuleName(BYTE rule)
{
	return (rule < entryRuleLast) ? szRuleNames[rule] : NULL;
}

HRESULT FilterPolicy::GetRuleDisplayName(BYTE ruleId, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (NULL == pszBuffer)
		return E_POINTER;

	*pszBuffer = TEXT('\0');

	INT stringId = 0;
	switch(ruleId)
	{
		case FilterPolicy::entryRuleAsk:		stringId = IDS_FILTERPOLICY_RULE_ASK; break;
		case FilterPolicy::entryRuleAdd:		stringId = IDS_FILTERPOLICY_RULE_ADD; break;
		case FilterPolicy::entryRuleIgnore:		stringId = IDS_FILTERPOLICY_RULE_IGNORE; break;
		case FilterPolicy::entryRuleEnumerate:	stringId = IDS_FILTERPOLICY_RULE_ENUMERATE; break;
	}
	
	if (0 == stringId)
		return E_INVALIDARG;
	else
		WASABI_API_LNGSTRINGW_BUF(stringId, pszBuffer, cchBufferMax);	
	
	return S_OK;
}

FilterPolicy *FilterPolicy::Clone()
{
	FilterPolicy *clone = new FilterPolicy();
	if (NULL != clone && listCount > 0)
	{			
		clone->list = (Entry*)malloc(sizeof(Entry) * listCount);
		if (NULL != clone->list)
		{
			clone->listSize = listCount;
			clone->listCount = listCount;
			CopyMemory(clone->list, list, sizeof(Entry) * listCount);
		}
	}
	return clone;
}

HRESULT FilterPolicy::Save(Profile *profile)
{
	TCHAR typeName[64];
	LPCTSTR ruleName;

	TCHAR szPath[MAX_PATH * 2];
	if (NULL == profile || FAILED(profile->GetFilePath(szPath, ARRAYSIZE(szPath))))
		return E_INVALIDARG;

	size_t remaining = listCount * (ARRAYSIZE(typeName) + 8) + 1;
	
	LPTSTR sectionString = (LPTSTR)malloc(sizeof(TCHAR) * remaining);
	if (NULL == sectionString)
		return E_OUTOFMEMORY;

	LPTSTR p = sectionString;
	HRESULT hr;
	
	if (NULL  != PLUGIN_REGTYPES)
	{
		for(INT i = 0; i < listCount; i++)
		{
			ruleName = GetRuleName(list[i].rule);
			IItemType *type = PLUGIN_REGTYPES->FindById(list[i].typeId);

			if (NULL != type && NULL != ruleName && 
				SUCCEEDED(type->GetName(typeName, ARRAYSIZE(typeName))))
			{
				hr = StringCchPrintfEx(p, remaining, &p, &remaining, 0, TEXT("%s=%s"), typeName, ruleName);
				if (0 == remaining)
					hr = STRSAFE_E_INSUFFICIENT_BUFFER;
				
				if (SUCCEEDED(hr))
				{
					p++;
					remaining--;
				}
				else 
					break;

			}	
		}
	}
	if (remaining < 2 && SUCCEEDED(hr))
		hr = STRSAFE_E_INSUFFICIENT_BUFFER;

	if (SUCCEEDED(hr))
	{
		for (INT i = 0; i < 2; i++, p++)
		{
			remaining--;
			*(p) = TEXT('\0');
		}
		WritePrivateProfileString(TEXT("FilterPolicy"), NULL, NULL, szPath);
		if (0 == WritePrivateProfileSection(TEXT("FilterPolicy"), sectionString, szPath))
			hr = HRESULT_FROM_WIN32(GetLastError());
	}

	free(sectionString);
	profile->Notify(ProfileCallback::eventFilterChanged);
	return hr;

}