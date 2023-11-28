#ifndef _NULLSOFT_WINAMP_DATAVIEW_LENGTH_UNIT_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_LENGTH_UNIT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./lengthConverter.h"


typedef enum UnitType
{	
	UnitType_Pixel = 0,
	UnitType_Dlu = 1,
	UnitType_Em = 2,
	UnitType_Point = 3,
	UnitType_Percent = 4,
} UnitType;

typedef struct LengthUnit
{
	float value;
	UnitType type;
} LengthUnit;

BOOL 
LengthUnit_Set(LengthUnit *unit, float value, UnitType type);

BOOL 
LengthUnit_Copy(LengthUnit *dst, const LengthUnit *src);

float 
LengthUnit_GetHorzPx(const LengthUnit *unit, const LengthConverter *converter);

float 
LengthUnit_GetVertPx(const LengthUnit *unit, const LengthConverter *converter);

int
LengthUnit_Compare(const LengthUnit *unit1, const LengthUnit *unit2, const LengthConverter *converter);

HRESULT
LengthUnit_Format(char *buffer, size_t bufferSize, const LengthUnit *unit);

HRESULT
LengthUnit_Parse(const char *string, UnitType defaultType, LengthUnit *unit);

#endif //_NULLSOFT_WINAMP_DATAVIEW_LENGTH_UNIT_HEADER