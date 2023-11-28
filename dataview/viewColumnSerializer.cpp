#include "main.h"
#include "./viewColumnSerializer.h"
#include "./lengthUnit.h"

#include "../nu/ptrlist.h"

#include <strsafe.h>

typedef nu::PtrList<ViewColumn> ColumnList;	

typedef struct ParserParameter
{
	ColumnList *columnList;
	size_t valueIndex;
	LengthUnit lengthUnit;
	BOOL lengthUnitValid;
	BOOL visible;
	char name[256];
	char buffer[128];
}ParserParameter;


ViewColumnSerializer::ViewColumnSerializer()
	: blockSeparator('|')
{
}

ViewColumnSerializer::~ViewColumnSerializer()
{
}

HRESULT ViewColumnSerializer::Serialize(char *buffer, size_t bufferSize, ViewColumnEnum *enumerator)
{
	HRESULT hr;
	char *cursor, *block;
	size_t remaining, length;
	ViewColumn *column;
	LengthUnit lengthUnit;


	if (NULL == buffer)
		return E_POINTER;

	buffer[0] = '\0';

	if (NULL == enumerator)
		return E_INVALIDARG;

	remaining = bufferSize;
	cursor = buffer;
	block = cursor;
	
	hr = S_OK;

	while(S_OK == enumerator->Next(&column, 1, NULL))
	{
		block = cursor;
		
		if (block != buffer)
		{
			if (remaining < 1)
			{
				hr = STRSAFE_E_INSUFFICIENT_BUFFER;
				break;
			}

			*cursor = blockSeparator;
			cursor++;
			remaining--;
		}
		
		hr = StringCchCopyExA(cursor, remaining, column->GetName(), &cursor, &remaining, 0);
		if (FAILED(hr))
			break;

		if (FAILED(column->GetWidth(&lengthUnit)))
		{
			hr = E_FAIL;
			break;
		}

		hr = LengthUnit_Format(cursor, remaining, &lengthUnit);
		if (FAILED(hr))
			break;
		
		length = lstrlenA(cursor);
		cursor += length;
		remaining -= length;

		if (FALSE == column->IsVisible())
		{
			hr = StringCchCopyExA(cursor, remaining, ",H", &cursor, &remaining, 0);
			if (FAILED(hr))
				break;
		}
	}
	
	if (FAILED(hr))
		*block = '\0';
	
	return hr;
}

static StringParserResponse __stdcall 
ViewColumnSerializer_ValueParserCb(const char *keyword, unsigned int length, const void *context)
{
	ParserParameter *self;
	

	self = (ParserParameter*)context;
	if (NULL == self)
		return StringParserResponse_Abort;
	
	switch (self->valueIndex)
	{
		case 0:
			if (FAILED(StringCchCopyNA(self->name, ARRAYSIZE(self->name), keyword, length)))
			{
				self->name[0] = '\0';
				return StringParserResponse_Abort;
			}
			break;
		case 1:
			if (SUCCEEDED(StringCchCopyNA(self->buffer, ARRAYSIZE(self->buffer), keyword, length)) &&
				SUCCEEDED(LengthUnit_Parse(self->buffer, UnitType_Pixel, &self->lengthUnit)))
			{
				self->lengthUnitValid = TRUE;
			}
			break;
		case 2:
			while(length--)
			{
				switch(keyword[length])
				{
					case L'H':
					case L'h':
						self->visible = FALSE;
						break;
				}
			}
			break;
	}

	self->valueIndex++;
	return StringParserResponse_Continue;
}

static StringParserResponse __stdcall 
ViewColumnSerializer_BlockParserCb(const char *keyword, unsigned int length, const void *context)
{
	ViewColumn *column;
	ParserParameter *self;

	self = (ParserParameter*)context;
	if (NULL == self || 0 == length)
		return StringParserResponse_Continue;

	self->valueIndex = 0;
	self->name[0] = '\0';
	self->lengthUnitValid = FALSE;
	self->visible = TRUE;
	
	AnsiString_ParseKeywords(keyword, length, ',', TRUE, ViewColumnSerializer_ValueParserCb, self);
	
	if (SUCCEEDED(ViewColumn::CreateInstance(self->name, &column)))
	{
		self->columnList->push_back(column);
		if (FALSE != self->lengthUnitValid)
			column->SetWidth(&self->lengthUnit);

		column->SetVisible(self->visible);
	}
	
	return StringParserResponse_Continue;
}


HRESULT ViewColumnSerializer::Deserialize(const char *buffer, size_t length, ViewColumnEnum **enumerator)
{
	HRESULT hr;
	ColumnList columnList;
	ParserParameter param;
	size_t index;
	
	param.columnList = &columnList;

	if (NULL == enumerator)
		return E_POINTER;

	if (NULL == buffer)
		return E_INVALIDARG;

	AnsiString_ParseKeywords(buffer, length, blockSeparator, TRUE, 
							ViewColumnSerializer_BlockParserCb, &param);

	hr = ViewColumnEnum::CreateInstance(columnList.begin(), columnList.size(), enumerator);

	
	index = columnList.size();
	while(index--)
	{
		columnList[index]->Release();
	}

	return hr;
}

void ViewColumnSerializer::SetBlockSeparator(char separator)
{
	blockSeparator = separator;
}

char ViewColumnSerializer::GetBlockSeparator()
{
	return blockSeparator;
}
