#ifndef NULLSOFT_ML_WATCHFILTER_HEADER
#define NULLSOFT_ML_WATCHFILTER_HEADER

#include ".\api_watchfilter.h"

class MLWatchFilter : public api_watchfilter
{
private: 
	typedef struct
	{
		unsigned int	length;
		unsigned int	allocated;
		unsigned int	count;
		wchar_t			*string;
	}FILTER_GROUP;	

public:
	MLWatchFilter(void);
	virtual ~MLWatchFilter(void);
	
public:
	int	AddFilter(const wchar_t *filter, int cchLen); // if length = -1 lstrlenW will be used
	void Clear(void);
	int	Check(const wchar_t *file, unsigned int cchLen);
	
	int Combine(api_watchfilter *source, int mode);

	int AddString(const wchar_t *filterString, wchar_t separator, wchar_t endChar);
	unsigned int GetStringLength(void);
	wchar_t* GetString(wchar_t *buffer, unsigned int cchLen);
	
	int IsEmpty(void) { return (0 == count); }

	int CopyTo(MLWatchFilter *destination);

protected:
	int AddFilterToGroup(FILTER_GROUP *group, const wchar_t *filter);
	const wchar_t *FindExtension(const wchar_t *extension, FILTER_GROUP *group);

					
private:
	void			*heap;			
	FILTER_GROUP	*filters;
	unsigned int	count;
	unsigned int	allocated;

protected:
		RECVS_DISPATCH;
};

#endif NULLSOFT_ML_WATCHFILTER_HEADER