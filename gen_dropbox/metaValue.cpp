#include "./lfHeap.h"
#include "./fileMetaInterface.h"

static HRESULT DuplicateWStr(LPWSTR *ppszOut, LPCWSTR pszSource)
{
	*ppszOut = NULL;
	if (NULL == pszSource) 	return S_OK;

	INT cbLen = (lstrlenW(pszSource) + 1) * sizeof(WCHAR);
	*ppszOut = (LPWSTR)lfh_malloc(cbLen);
	if (NULL == *ppszOut) 
		return E_OUTOFMEMORY;
	CopyMemory(*ppszOut, pszSource, cbLen);

	return S_OK;
}

void ReleaseMetaValue(METAVALUE *pValue)
{
	if (NULL == pValue) 
		return;
	switch(pValue->type)
	{
		case METATYPE_BSTR:
			SysFreeString(pValue->bstrVal);
			break;
		case METATYPE_WSTR:
			lfh_free(pValue->pwVal);
			break;
	}
}

HRESULT DuplicateMetaValue(METAVALUE *pValueOut, const METAVALUE *pValueIn)
{
	pValueOut->type = pValueIn->type;
	switch(pValueIn->type)
	{
		case METATYPE_BSTR:
			pValueOut->bstrVal = SysAllocStringLen(pValueIn->bstrVal, SysStringLen(pValueIn->bstrVal));
			return S_OK;

		case METATYPE_WSTR:
			return DuplicateWStr(&pValueOut->pwVal, pValueIn->pwVal);

		case METATYPE_INT32:
			pValueOut->iVal = pValueIn->iVal;
			return S_OK;
	}
	return E_FAIL;
}

HRESULT MetaValueWStrW(METAVALUE *pValue, LPCWSTR pszStringIn)
{
	pValue->type = METATYPE_WSTR;
	return DuplicateWStr(&pValue->pwVal, pszStringIn);
}

HRESULT MetaValueInt32(METAVALUE *pValue, INT nValueIn)
{
	pValue->type = METATYPE_INT32;
	pValue->iVal = nValueIn;
	return S_OK;
}

HRESULT MetaValueEmpty(METAVALUE *pValue)
{
	pValue->type = METATYPE_EMPTY;
	pValue->iVal = 0;
	pValue->pwVal = NULL;
	return S_OK;
}

