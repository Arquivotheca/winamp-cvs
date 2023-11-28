#include "main.h"
#include "./lengthUnit.h"
#include <strsafe.h>

static inline int 
LengthUnit_CompareFloats(float value1, float value2)
{
	if (value1 > value2) 
		return 1;
	if (value1 < value2)
		return -1;
	
	return 0;
}

BOOL 
LengthUnit_Set(LengthUnit *unit, float value, UnitType type)
{
	if (NULL == unit)
		return FALSE;

	unit->type = type;
	unit->value = value;
	return TRUE;
}

BOOL 
LengthUnit_Copy(LengthUnit *dst, const LengthUnit *src)
{
	if (NULL == dst || NULL == src)
		return FALSE;

	dst->type = src->type;
	dst->value = src->value;

	return TRUE;
}

float
LengthUnit_GetHorzPx(const LengthUnit *unit, const LengthConverter *converter)
{
	if (NULL == unit)
		return 0.0f;

	switch(unit->type)
	{
		case UnitType_Pixel:
			return unit->value;
		case UnitType_Dlu:
			return LengthConverter_DluToPixelsX(converter, unit->value);
		case UnitType_Em:
			return LengthConverter_EmToPixelsX(converter, unit->value);
		case UnitType_Point:
			return LengthConverter_PointsToPixelsX(converter, unit->value);
		case UnitType_Percent:
			return LengthConverter_PercentToPixelsX(converter, unit->value);
	}

	return 0.0f;
}

float
LengthUnit_GetVertPx(const LengthUnit *unit, const LengthConverter *converter)
{
	if (NULL == unit)
		return 0.0f;

	switch(unit->type)
	{
		case UnitType_Pixel:
			return unit->value;
		case UnitType_Dlu:
			return LengthConverter_DluToPixelsY(converter, unit->value);
		case UnitType_Em:
			return LengthConverter_EmToPixelsY(converter, unit->value);
		case UnitType_Point:
			return LengthConverter_PointsToPixelsY(converter, unit->value);
		case UnitType_Percent:
			return LengthConverter_PercentToPixelsY(converter, unit->value);
	}

	return 0.0f;
}

int
LengthUnit_Compare(const LengthUnit *unit1, const LengthUnit *unit2, const LengthConverter *converter)
{
	if (NULL == unit1 || NULL == unit2)
		return (NULL != unit1) ? 1 : ((NULL != unit2) ? -1 : 0);

	if (unit1->type != unit2->type)
	{
		float value1, value2;
		int result;

		value1 = LengthUnit_GetHorzPx(unit1, converter);
		value2 = LengthUnit_GetHorzPx(unit2, converter);
		result = LengthUnit_CompareFloats(value1, value2);

		if (0 == result)
		{
			value1 = LengthUnit_GetVertPx(unit1, converter);
			value2 = LengthUnit_GetVertPx(unit2, converter);
			result = LengthUnit_CompareFloats(value1, value2);
		}
		return result;
	}

	return LengthUnit_CompareFloats(unit1->value, unit2->value);
}

HRESULT
LengthUnit_Format(char *buffer, size_t bufferSize, const LengthUnit *unit)
{
	const char *suffix;

	if (NULL == buffer)
		return E_POINTER;

	*buffer = '\0';
	if (NULL == unit)
		return E_INVALIDARG;

	switch(unit->type)
	{
		case UnitType_Pixel:
			suffix = "px";
			break;
		case UnitType_Dlu:
			suffix = "dlu";
			break;
		case UnitType_Em:
			suffix = "em";
			break;
		case UnitType_Point:
			suffix = "pt";
			break;
		case UnitType_Percent:
			suffix = "%";
			break;
		default:
			suffix = NULL;
			break;
	}

	if (NULL == suffix)
		return E_UNEXPECTED;

	return StringCchPrintfA(buffer, bufferSize, "%.2f%s", unit->value, suffix);
}

HRESULT
LengthUnit_Parse(const char *string, UnitType defaultType, LengthUnit *unit)
{
	char suffix[8];
	int found;

	if (NULL == unit)
		return E_POINTER;

	if (NULL == string || '\0' == *string)
	{
		unit->type = defaultType;
		unit->value = 0.0f;
		return S_OK;
	}
	
	found = sscanf_s(string, "%f%4s", &unit->value, suffix, ARRAYSIZE(suffix));
	if(0 == found)
		return E_FAIL;

	if (1 == found)
	{
		unit->type = defaultType;
		return S_OK;
	}
	
	if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "m", -1, suffix, -1))
	{
		unsigned int length;
		length = lstrlenA(string);
		if (length > 1 && 
			CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "em", -1, (string + (length - 2)), -1))
		{
			StringCchCopyA(suffix, ARRAYSIZE(suffix), "em");
		}
	}

	if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "px", -1, suffix, -1))
		unit->type = UnitType_Pixel;
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "dlu", -1, suffix, -1))
		unit->type = UnitType_Dlu;
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "em", -1, suffix, -1))
		unit->type = UnitType_Em;
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "pt", -1, suffix, -1))
		unit->type = UnitType_Point;
	else if (CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, "%", -1, suffix, -1))
		unit->type = UnitType_Percent;
	else 
		return E_FAIL;
		
	return S_OK;
}