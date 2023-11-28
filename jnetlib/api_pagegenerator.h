#pragma once
#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

class NOVTABLE api_pagegenerator : public Dispatchable
{
public:
	api_pagegenerator() {}
	~api_pagegenerator() {}
	DISPATCH_CODES {
		API_PAGEGENERATOR_GETDATA=10,
		API_PAGEGENERATOR_ISNONBLOCKING=20,
	};
	int GetData(char *buf, int size);
	int IsNonBlocking();
	void destruct();
};
inline int api_pagegenerator::GetData(char *buf, int size)
{
	return _call(API_PAGEGENERATOR_GETDATA, (int)0, buf, size);
}

inline int api_pagegenerator::IsNonBlocking()
{
	return _call(API_PAGEGENERATOR_ISNONBLOCKING, (int)0);
}