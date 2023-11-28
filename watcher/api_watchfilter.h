#ifndef __WASABI_API_WATCHFILTER_H
#define __WASABI_API_WATCHFILTER_H

#include <bfc/dispatch.h>
#include <bfc/platform/types.h>

#define COMBINEMODE_INTERSECT	0
#define COMBINEMODE_EXCLUDE		1
#define COMBINEMODE_JOIN			2

// {A71A8738-C4BF-11DA-9B41-B622A1EF5492}
static const GUID watchfilterGUID = 
{ 0xA71A8738, 0xC4BF, 0x11DA, { 0x9B, 0x41, 0xB6, 0x22, 0xA1, 0xEF, 0x54, 0x92} };

class NOVTABLE api_watchfilter : public Dispatchable
{
protected:
	api_watchfilter() {}
	~api_watchfilter() {}
public:
	int AddString(const wchar_t *string, wchar_t separator, wchar_t endChar);
	int	Check(const wchar_t *file, unsigned int cchLen);
	int IsEmpty(void);
	void Clear(void);
	int Combine(api_watchfilter *source, int mode);
	api_watchfilter* CopyTo(api_watchfilter *destination);

	unsigned int GetStringLength(void);
	wchar_t *GetString(wchar_t *buffer, unsigned int cchLen);

	DISPATCH_CODES
	{
	    API_WATCHFILTER_ADDSTRING		= 0x0010,
		API_WATCHFILTER_CHECK			= 0x0020,
		API_WATCHFILTER_ISEMTPY			= 0x0030,
	    API_WATCHFILTER_CLEAR			= 0x0040,
		API_WATCHFILTER_COMBINE			= 0x0050,

		API_WATCHFILTER_COPYTO			= 0x0100,

		API_WATCHFILTER_GETSTRING		= 0x0200,
		API_WATCHFILTER_GETSTRINGLENGTH	= 0x0210	,	
	};
};

inline int api_watchfilter::AddString(const wchar_t *string, wchar_t separator, wchar_t endChar)
{
	return _call(API_WATCHFILTER_ADDSTRING, (int)0, string, separator, endChar);
}
inline int api_watchfilter::Check(const wchar_t *file, unsigned int cchLen)
{
	return _call(API_WATCHFILTER_CHECK, (int)NULL,  file, cchLen);
}
inline int api_watchfilter::Combine(api_watchfilter *source, int mode)
{
	return _call(API_WATCHFILTER_COMBINE, (int)NULL,  source, mode);
}
inline api_watchfilter* api_watchfilter::CopyTo(api_watchfilter *destination)
{
	return _call(API_WATCHFILTER_COPYTO, (api_watchfilter*)NULL,  destination);
}

inline unsigned int api_watchfilter::GetStringLength(void)
{
	return _call(API_WATCHFILTER_GETSTRINGLENGTH, (unsigned int)NULL);
}
inline wchar_t* api_watchfilter::GetString(wchar_t *buffer, unsigned int cchLen)
{
	return _call(API_WATCHFILTER_GETSTRING, (wchar_t*)NULL,  buffer, cchLen);
}

inline void api_watchfilter::Clear(void)
{
	_voidcall(API_WATCHFILTER_CLEAR);
}

inline int api_watchfilter::IsEmpty(void)
{
	return _call(API_WATCHFILTER_ISEMTPY, (int)0);
}
#endif // __WASABI_API_WATCHFILTER_H