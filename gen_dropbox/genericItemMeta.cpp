#include "./genericItemMeta.h"
#include "../nu/trace.h"


static int __cdecl GenericItemMeta_SearchCompare(const void *key, const void *elem)
{
	return ((METAKEY)(INT_PTR)key) - *((METAKEY*)elem);
}

static int __cdecl GenericItemMeta_SortCompare(const void *elem1, const void *elem2)
{
	return *((METAKEY*)elem1) - *((METAKEY*)elem2);
}


typedef struct __METAVALUEDIRECT
{
	WORD state;
	BYTE type;
	union
	{
		INT iVal;			// METATYPE_I32
		LPWSTR pwVal;		// METATYPE_WSTR
		BSTR  bstrVal;
	};
	
} METAVALUEDIRECT;

GenericItemMeta::GenericItemMeta()
{	
	SecureZeroMemory(szDirectRecords, ARRAYSIZE(szDirectRecords) * sizeof(DIRECTRECORD));
}

GenericItemMeta::~GenericItemMeta()
{
	size_t index;
	for (index = 0; index < ARRAYSIZE(szDirectRecords); index++)
	{
		if (0 != (METARECORD_SET & szDirectRecords[index].state))
			ReleaseMetaValue(&szDirectRecords[index].value);
	}

	index = exRecords.size();
    while(index--)
	{
		ReleaseMetaValue(&exRecords[index].value);
	}
}

STDMETHODIMP GenericItemMeta::Clear()
{
	size_t index;
	for (index = 0; index < ARRAYSIZE(szDirectRecords); index++)
	{
		if (0 != (METARECORD_SET & szDirectRecords[index].state))
			ReleaseMetaValue(&szDirectRecords[index].value);
	}
	SecureZeroMemory(szDirectRecords, ARRAYSIZE(szDirectRecords)*sizeof(DIRECTRECORD));

	index = exRecords.size();
    while(index--)
	{
		ReleaseMetaValue(&exRecords[index].value);
	}
	exRecords.clear();

	return S_OK;
}

STDMETHODIMP GenericItemMeta::GetState(METAKEY metaKey, INT *pState)
{
	if (0 == metaKey)
		return E_INVALIDARG;

	if (metaKey <= ARRAYSIZE(szDirectRecords))
	{
		DIRECTRECORD *pdr = &szDirectRecords[metaKey - 1];
		if (0 == (METARECORD_SET & pdr->state))
			return E_METAKEY_UNKNOWN;
		
		*pState = pdr->state;
		return S_OK;
	}

	EXRECORD *pRecord = (EXRECORD*)bsearch((INT*)(INT_PTR)metaKey, exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SearchCompare);
	if (NULL == pRecord)
		return E_METAKEY_UNKNOWN;

	*pState = pRecord->state;
	return S_OK;
}
STDMETHODIMP GenericItemMeta::QueryValue(METAKEY metaKey, METAVALUE *pMetaValue)
{
	return QueryValueReal(metaKey, pMetaValue, NULL, NULL, TRUE);
}

STDMETHODIMP GenericItemMeta::QueryValueHere(METAKEY metaKey, METAVALUE *pMetaValue, VOID *pBuffer, size_t cbBuffer)
{
	if (NULL == pBuffer || 0 == cbBuffer)
		return E_POINTER;
	return QueryValueReal(metaKey, pMetaValue, pBuffer, cbBuffer, TRUE);
}


STDMETHODIMP GenericItemMeta::SetValue(METAKEY metaKey, METAVALUE *pMetaValue, BOOL bMakeCopy)
{
	HRESULT hr = S_OK;
	if (NULL == pMetaValue || 0 == metaKey)
		return E_INVALIDARG;

	FILEMETARECORD tempRec;
	tempRec.key = metaKey;
	CopyMemory(&tempRec.value, pMetaValue, sizeof(METAVALUE));
	
	INT addFlags = ADDMETAFLAG_MARKMODIFIED;
	if (bMakeCopy) addFlags |= ADDMETAFLAG_MAKECOPY;

	return AddMetaRecords(&tempRec, 1, METAREADMODE_REPLACE, addFlags);
}

STDMETHODIMP GenericItemMeta::SetRecords(FILEMETARECORD *pRecords, size_t count, INT readMode, BOOL bMakeCopy, BOOL bMarkModified)
{
	HRESULT hr = S_OK;
	if (NULL == pRecords || 0 == count)
		return E_INVALIDARG;

	INT addFlags = 0;
	if (bMarkModified) addFlags |= ADDMETAFLAG_MARKMODIFIED;
	if (bMakeCopy) addFlags |= ADDMETAFLAG_MAKECOPY;

	return AddMetaRecords(pRecords, count, readMode, addFlags);
}

STDMETHODIMP GenericItemMeta::RemoveValue(METAKEY metaKey)
{
	if (metaKey <= ARRAYSIZE(szDirectRecords))
	{
		DIRECTRECORD *pdr = &szDirectRecords[metaKey - 1];
		if (0 == (METARECORD_SET & pdr->state))
			return E_METAKEY_UNKNOWN;
		ReleaseMetaValue(&pdr->value);
		pdr->state = 0;
		return S_OK;
	}

	EXRECORD *pRecord = (EXRECORD*)bsearch((INT*)(INT_PTR)metaKey, exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SearchCompare);
	if (NULL == pRecord)
		return E_METAKEY_UNKNOWN;
	
	ReleaseMetaValue(&pRecord->value);
	exRecords.erase(pRecord);
	qsort(exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SortCompare);
	return S_OK;
}

STDMETHODIMP GenericItemMeta::GetReader(METAKEY *metaKey, INT metaCount, INT readMode, IFileMetaReader **ppMetaReader)
{
	return E_NOTIMPL;
}

STDMETHODIMP GenericItemMeta::CanRead(METAKEY metaKey)
{
	return E_NOTIMPL;
}

STDMETHODIMP GenericItemMeta::FilterKeys(METAKEY *metaKey, INT *keyCount, INT stateFilter)
{
	if (NULL == metaKey || NULL == keyCount)
		return E_POINTER;

	INT maxCount = *keyCount;
	INT filteredCount = 0;

	
	for (int i = 0; i < maxCount; i++)
	{
		if (metaKey[i] <= ARRAYSIZE(szDirectRecords))
		{
			DIRECTRECORD *pdr = &szDirectRecords[metaKey[i]- 1];
			if (0 == (METARECORD_SET & pdr->state) ||
				(stateFilter & ~METARECORD_SET) == (pdr->state & ~METARECORD_SET))
			{
				metaKey[filteredCount] = metaKey[i];
				filteredCount++;
			}
		}
		else if (((SHORT)metaKey[i]) > 0)
		{
			size_t index = exRecords.size();
			BOOL notFound = TRUE;
			while(index--)
			{
				if (exRecords[index].key == metaKey[i])
				{
					if (stateFilter == exRecords[index].state)
					{
						metaKey[filteredCount] = metaKey[i];
						filteredCount++;
					}
					notFound = FALSE;
					break;
				}
			}
			if (notFound)
			{
				metaKey[filteredCount] = metaKey[i];
				filteredCount++;
			}
		}

	}


	*keyCount = filteredCount;
	return S_OK;
}

BOOL GenericItemMeta::NeedRead(METAKEY *keyList, INT keyCount)
{
	for (int i = 0; i < keyCount; i++)
	{		
		if (keyList[i] <= ARRAYSIZE(szDirectRecords))
		{
			DIRECTRECORD *pdr = &szDirectRecords[keyList[i]- 1];
			if (0 == ((METARECORD_READING | METARECORD_WRITING | METARECORD_MODIFIED) & pdr->state) &&
				(0 == (METARECORD_SET & pdr->state) || METATYPE_EMPTY == pdr->value.type))
			{				
				return TRUE;
			}
		}
		else if (((SHORT)keyList[i]) > 0)
		{
			size_t index = exRecords.size();
			BOOL notFound = TRUE;
			while(index--)
			{
				if (exRecords[index].key == keyList[i])
				{
					notFound = FALSE;
					break;
				}
			}
			if (notFound)
			{
				return TRUE;
			}
		}

	}

	return FALSE;
}
void GenericItemMeta::SetState(METAKEY *keyList, size_t keyCount, UINT state, BOOL bSet, BOOL bForceAdd)
{	
	for (size_t i = 0; i < keyCount; i++)
	{		
		if (keyList[i] <= ARRAYSIZE(szDirectRecords))
		{
			DIRECTRECORD *pdr = &szDirectRecords[keyList[i] - 1];
			if (0 == (METARECORD_SET & pdr->state))
			{
				if (bForceAdd)
				{
					pdr->state = METARECORD_SET;
					MetaValueEmpty(&pdr->value);
				}
				else 
					continue;
			}
			
			if (bSet) pdr->state |= state;
			else pdr->state &= ~state;
		}
		else if (((SHORT)keyList[i]) > 0)
		{
			size_t index = exRecords.size();
			EXRECORD *per = NULL;
			while(index--)
			{
				if (exRecords[index].key == keyList[i])
				{
					per = &exRecords[index];
					break;
				}
			}
			if (NULL == per)
				continue;
			
			if (bSet) per->state |= state;
			else per->state &= ~state;
		}

	}
}

HRESULT GenericItemMeta::AddMetaRecords(FILEMETARECORD *pRecords, size_t count, INT readMode, INT addFlags)
{
	size_t metaRecordsSize = exRecords.size();
	size_t copied = 0;
	BOOL sortRequired = FALSE;
	HRESULT hr = S_OK;

	if (NULL == pRecords)
	{
		return E_POINTER;
	}

	for(size_t i = 0; i < count; i++)
	{
		EXRECORD *pRecord;
		if (pRecords[i].key <= ARRAYSIZE(szDirectRecords))
		{
			DIRECTRECORD *pdr = &szDirectRecords[pRecords[i].key - 1];
			if (0 == (METARECORD_SET & pdr->state))
			{
				pdr->state = METARECORD_SET;
				pdr->value = pRecords[i].value;
			}
			else
			{
				if (METATYPE_EMPTY != pdr->value.type) 
				{
					if (METAREADMODE_NORMAL == readMode)
					{
						if (0 == (ADDMETAFLAG_MAKECOPY & addFlags))
						{
							ReleaseMetaValue(&pRecords[i].value);
						}
						continue;
					}
					else
					{
						ReleaseMetaValue(&pdr->value);
					}
				}
			}

			pdr->state &= ~(METARECORD_MODIFIED | METARECORD_READ);
			if (0 != ((ADDMETAFLAG_MARKREAD | ADDMETAFLAG_MARKMODIFIED)& addFlags))
				pdr->state |= METARECORD_MODIFIED;
						
			if (0 != (ADDMETAFLAG_MAKECOPY & addFlags))
			{
				hr = DuplicateMetaValue(&pdr->value, &pRecords[i].value);
				if (FAILED(hr)) break;
			}
			else
				CopyMemory(&pdr->value, &pRecords[i].value, sizeof(METAVALUE));
		}
		else if (((SHORT)pRecords[i].key) > 0)
		{
			pRecord = (EXRECORD*)bsearch((INT*)(INT_PTR)pRecords[i].key, exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SearchCompare);

			if (NULL == pRecord)
			{
				size_t capacity = exRecords.capacity();
				if (capacity == (exRecords.size() - 1))
					exRecords.reserve((capacity + (count - i))*2);
				EXRECORD er;
				er.key	 = pRecords[i].key;
				er.value = pRecords[i].value;
				er.state = METARECORD_SET;
				exRecords.push_back(er);

				pRecord = &exRecords.back();
				if (!sortRequired)
					sortRequired = TRUE;
			}
			else 
			{
				if (METAREADMODE_NORMAL == readMode &&
					METATYPE_EMPTY != pRecord->value.type) 
				{
					if (0 == (ADDMETAFLAG_MAKECOPY & addFlags))
						ReleaseMetaValue(&pRecords[i].value);
					continue;
				}

				pRecord->key = pRecords[i].key;
				ReleaseMetaValue(&pRecord->value);
			}

			pRecord->state &= ~(METARECORD_MODIFIED | METARECORD_READ);
			if (0 != ((ADDMETAFLAG_MARKREAD | ADDMETAFLAG_MARKMODIFIED)& addFlags))
				pRecord->state |= METARECORD_MODIFIED;
			

			if (0 != (ADDMETAFLAG_MAKECOPY & addFlags))
			{
				hr = DuplicateMetaValue(&pRecord->value, &pRecords[i].value);
				if (FAILED(hr)) break;
			}
			else
				CopyMemory(&pRecord->value, &pRecords[i].value, sizeof(METAVALUE));
		}
		
		copied++; 
	}

	if (FAILED(hr))
	{ // rollback;
		for(size_t index = exRecords.size(); index > metaRecordsSize ; index--)
			exRecords.pop_back();
	}
	else
	{
		if (sortRequired)
			qsort(exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SortCompare);
	}

	return S_OK;
}

HRESULT GenericItemMeta::QueryValueReal(METAKEY metaKey, METAVALUE *pMetaValue, VOID *pBuffer, size_t cbBuffer, BOOL bStrict)
{
	HRESULT hr = S_OK;
	size_t cbLen;
	METAVALUE *pValue = NULL;

	if (NULL == pMetaValue || ((SHORT)metaKey) < 1)
		return E_INVALIDARG;

	if (metaKey <= ARRAYSIZE(szDirectRecords))
	{
		DIRECTRECORD *pdr = &szDirectRecords[metaKey - 1];
		if (0 == (METARECORD_SET & pdr->state))
			hr = E_METAKEY_UNKNOWN;
		else
			pValue = &pdr->value;
	}
	else
	{
		EXRECORD *pRecord = (EXRECORD*)bsearch((INT*)(INT_PTR)metaKey, exRecords.begin(), exRecords.size(), sizeof(EXRECORD), GenericItemMeta_SearchCompare);
		if (NULL == pRecord)
			hr = E_METAKEY_UNKNOWN;
		else
			pValue = &pRecord->value;
	}

	if (NULL != pValue)
	{
		if (pMetaValue->type != pValue->type && bStrict)
			hr = E_METAVALUE_BADFORMAT;
		else
		{
			pMetaValue->type = pValue->type;
			if (NULL == pBuffer && 0 == cbBuffer)
				hr = DuplicateMetaValue(pMetaValue, pValue);
			else
			{
				if (NULL == pBuffer)
					return E_POINTER;
				switch(pValue->type)
				{
					case METATYPE_INT32:
						if (cbBuffer < sizeof(INT32))
						{
							pMetaValue->iVal = 0;
							hr = E_OUTOFMEMORY;
						}
						else
						{
							*((INT*)pBuffer) = pValue->iVal;
							pMetaValue->iVal = pValue->iVal;
						}
						break;
					
					case METATYPE_WSTR:
						cbLen = ((NULL != pValue->pwVal) ? lstrlenW(pValue->pwVal) : 0) * sizeof(WCHAR);
						if (cbBuffer < cbLen + sizeof(WCHAR))
						{
							pMetaValue->pwVal = NULL;
							if (cbBuffer >= sizeof(WCHAR))
								*((WCHAR*)pBuffer) = L'\0';
							hr = E_OUTOFMEMORY;
						}
						else
						{
							if (0 == cbLen)
								*((WCHAR*)pBuffer) = L'\0';
							else
								CopyMemory(pBuffer, pValue->pwVal, (cbLen + sizeof(WCHAR)));
							pMetaValue->pwVal = (WCHAR*)pBuffer;
						}
						break;

					case METATYPE_BSTR:
						hr = E_METAVALUE_BADFORMAT;
						break;

					default:
						hr = E_METAVALUE_BADFORMAT;
						break;
				}
			}
		}

	}
	return hr;
}