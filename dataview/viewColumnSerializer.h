#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_SERIALIZER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_SERIALIZER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./viewColumn.h"
#include "./viewColumnEnum.h"

class ViewColumnSerializer
{
public:
	ViewColumnSerializer();
	~ViewColumnSerializer();

public:
	HRESULT Serialize(char *buffer, 
					  size_t bufferSize, 
					  ViewColumnEnum *enumerator);

	HRESULT Deserialize(const char *buffer, 
						size_t length, 
						ViewColumnEnum **enumerator);

	void SetBlockSeparator(char separator);
	char GetBlockSeparator();


private:
	char blockSeparator;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_SERIALIZER_HEADER