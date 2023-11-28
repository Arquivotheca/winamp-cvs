#include <windows.h>
#include <strsafe.h>
#include ".\watchfilter.h"

#define FILTER_ALLOCSTEP		4
#define FILTERGROUP_ALLOCSTEP	4

#define FILTER_SEPARATOR		L';'
	

MLWatchFilter::MLWatchFilter(void) : heap(NULL), filters(NULL), count(0), allocated(0)
{
	heap = GetProcessHeap();
}

MLWatchFilter::~MLWatchFilter(void)
{
	Clear();
}

int	MLWatchFilter::AddFilter(const wchar_t *filter,  int cchLen)
{
	if (!heap || !filter) return 0;
	
	unsigned int len = (cchLen == -1) ? lstrlenW(filter) : cchLen;
	for(unsigned int i = 0; i < count; i++)
	{
		if (filters[i].length == len)
		{
			FILTER_GROUP *group = &filters[i];
			return AddFilterToGroup(group, filter);
		}
	}
	// we didn't have this group
	if (allocated <= count)
	{
		do { allocated += FILTER_ALLOCSTEP; } while (allocated <= count);
		filters = (FILTER_GROUP*) ((NULL==filters) ? HeapAlloc(heap, HEAP_ZERO_MEMORY, allocated*(sizeof(FILTER_GROUP))) :
											HeapReAlloc(heap, HEAP_ZERO_MEMORY, filters, allocated*(sizeof(FILTER_GROUP))));
	}
	filters[count].length = len;
    int ret = AddFilterToGroup(&filters[count], filter);
	if(ret) count++;
	return ret;
}

void MLWatchFilter::Clear(void)
{
	if (allocated)
	{
		for(unsigned int i = 0; i < count; i++) HeapFree(heap, NULL, filters[i].string);
		HeapFree(heap, NULL, filters);
		filters = NULL;
		allocated = NULL;
		count = 0;
	}
}

int	MLWatchFilter::Check(const wchar_t *file, unsigned int cchLen)
{
	unsigned int extLen = cchLen;
	while ((file[extLen] != L'.') && ((extLen--) > 0));
	if (extLen == 0) return 0;
	extLen = cchLen - extLen - 1;
	for(unsigned int i = 0; i < count; i++)
	{
		if (filters[i].length == extLen) return (NULL != FindExtension(file + (cchLen - extLen), &filters[i]));
	}
	return 0;

}

int MLWatchFilter::AddFilterToGroup(FILTER_GROUP *group, const wchar_t *filter)
{// if filter already in the group it will be skipped

	unsigned int len = (group->count + 1)*group->length;
	if (group->allocated <= len) // always reserve at least +1 filter
	{
		do { group->allocated += (FILTERGROUP_ALLOCSTEP*group->length); } while (group->allocated <= len);
		group->string = (wchar_t*) ((NULL == group->string) ? HeapAlloc(heap, NULL, group->allocated*(sizeof(wchar_t*))) :
										HeapReAlloc(heap, NULL, group->string, group->allocated*(sizeof(wchar_t*))));
	}
	 
	wchar_t *data;
	data = group->string + group->count*group->length;
	CopyMemory(data, filter, group->length*sizeof(wchar_t));
	CharUpperBuffW(data, group->length);
	
	//sorting
	unsigned int offset = 0;
	wchar_t	*position = NULL;
	unsigned int strLen = group->count*group->length;
	for (unsigned int idx = 0; idx < strLen; idx += group->length)
	{
		if (group->string[idx] < data[offset]) continue;
		if (group->string[idx] == data[offset])
		{
			do 	{ idx++; offset++; } while (group->string[idx] == data[offset] && (offset < group->length));
			if (offset == group->length) return 1; // filter already in the group - skip adding it 
			if (group->string[idx] < data[offset]) { idx -= offset; offset = 0; continue; }
		}
		// our postition
		position = &group->string[idx] - offset;
		break;
	}
	if (position) // move if neccessary
	{
		CopyMemory(data + group->length, data, group->length*sizeof(wchar_t));
		MoveMemory(position + group->length, position, (group->count*group->length - (position - group->string))*sizeof(wchar_t));
		CopyMemory(position, data + group->length, group->length*sizeof(wchar_t));
	}

	group->count++;
	return 1;
}

int MLWatchFilter::AddString(const wchar_t *filterString, wchar_t separator, wchar_t endChar)
{
	if (!filterString || filterString[0] == endChar) return 1;
	
	const wchar_t *current = filterString;
	const wchar_t *filter = current;
	do
	{
		if (*current == separator || endChar == *current)
		{
			if (current != filter)
			{
				if (!AddFilter(filter, (int)(current - filter))) return 0;
			}
			if (*current == endChar) break;
			current++;
			if (*current == endChar) break;
			filter = current;
		}
		
	}while(current++);
	return 1;
}
unsigned int MLWatchFilter::GetStringLength(void)
{
	unsigned int totalLen = 0;
	for(unsigned int i = 0; i < count; i++) totalLen += filters[i].count * (filters[i].length + 1);
	return (totalLen != 0) ? totalLen : 0;

}

wchar_t* MLWatchFilter::GetString(wchar_t *buffer, unsigned int cchLen)
{
	if (!buffer) return NULL;
	unsigned int len = 0;
	buffer[0] = 0x0000;
	wchar_t *current = buffer;
	for(unsigned int i = 0; i < count; i++)
	{
		unsigned int size = filters[i].length * sizeof(wchar_t);	
		for (unsigned int k = 0; k < filters[i].count; k++)
		{
			if (cchLen < (filters[i].length + 1)) break;  // not enough space
			CopyMemory(current, filters[i].string + k*filters[i].length, size);
			current +=filters[i].length;
			*current = FILTER_SEPARATOR;
			current++;
			cchLen -= (filters[i].length + 1);
		}
	}
	if (buffer < current) *(current-1)= 0x0000;
	return buffer;
}
int MLWatchFilter::CopyTo(MLWatchFilter *destination)
{
	destination->Clear();
	destination->allocated = allocated;
	destination->count = count;

	destination->filters = (allocated) ? (FILTER_GROUP*)HeapAlloc(destination->heap, HEAP_ZERO_MEMORY, destination->allocated*(sizeof(FILTER_GROUP))) : NULL;

	for(unsigned int i = 0; i < count; i++)
	{
		destination->filters[i].count = filters[i].count;
		destination->filters[i].allocated = filters[i].allocated;	
		destination->filters[i].length = filters[i].length;
		destination->filters[i].string = (wchar_t*) HeapAlloc(destination->heap, NULL, destination->filters[i].allocated*sizeof(wchar_t));
		CopyMemory(destination->filters[i].string, filters[i].string, filters[i].allocated*sizeof(wchar_t));
	}
	return 1;
}

const wchar_t* MLWatchFilter::FindExtension(const wchar_t *extension, FILTER_GROUP *group)
{
	
	unsigned int offset = 0;
	wchar_t test = extension[offset];
	CharUpperBuffW(&test, 1);

	unsigned int strLen = group->count*group->length;
	for (unsigned int idx = 0; idx < strLen; idx += group->length)
	{
		if (group->string[idx] < test) continue;
		if (group->string[idx] == test)
		{
			do 
			{ 
				idx++; offset++;
				test = extension[offset];
				CharUpperBuffW(&test, 1);
			} while (group->string[idx] == test && (offset < group->length));
			if (offset == group->length) return &group->string[idx - offset];
			if (group->string[idx] < test) 
			{ 
				idx -= offset; 
				offset = 0;
				test = group->string[idx];  // cause we know they was the same
				continue;
			}
		}
		return NULL;
	}
	return NULL;
}

int MLWatchFilter::Combine(api_watchfilter *source, int mode)
{
	// if there is no intersection - old value are stays
	
	MLWatchFilter *src = (MLWatchFilter*) source;
	wchar_t *tmpStr = NULL;
	unsigned int tmpLen = 0;
	unsigned int tmpAlloc = 0;

	if (COMBINEMODE_JOIN == mode)
	{
		tmpAlloc = src->GetStringLength();
		tmpStr = (wchar_t*)HeapAlloc(heap, NULL, tmpAlloc*sizeof(wchar_t));
		src->GetString(tmpStr, tmpAlloc);
	}
	else
	{
		const wchar_t *found = NULL;

		for(unsigned int i = 0; i < count; i++) // each my group
		{
			for(unsigned int j = 0; j < src->count; j++)
			{
				if (filters[i].length == src->filters[j].length)
				{ // we have same length group lets lool for extebsions
					for(unsigned int k = 0; k < filters[i].count; k++)
					{
						found = FindExtension(filters[i].string + k*filters[i].length, &(src->filters[j]));
						if (COMBINEMODE_INTERSECT == mode && !found) continue; // doesn't interset
						else if (COMBINEMODE_EXCLUDE == mode)
						{
							if(found) continue; // doesn't interset
							found = filters[i].string + k*filters[i].length;
						}

						if (tmpAlloc <= tmpLen + filters[i].length + 1)
						{
							do { tmpAlloc += 1024; } while (tmpAlloc  <= tmpLen + filters[i].length + 1);
							tmpStr = (wchar_t*) ((!tmpStr) ? HeapAlloc(heap, NULL, tmpAlloc*sizeof(wchar_t)) : 
															HeapReAlloc(heap, NULL, tmpStr, tmpAlloc*sizeof(wchar_t)));

						}
						CopyMemory(tmpStr + tmpLen, found, filters[i].length*sizeof(wchar_t));
						tmpLen += filters[i].length + 1;
						tmpStr[tmpLen - 1] = FILTER_SEPARATOR;
					}
				}
			}
		}
		if (tmpStr && tmpLen > 0)
		{
			tmpStr[tmpLen - 1] = 0x0000;
			Clear();
		}
	}

	BOOL retCode = (tmpStr) ? AddString(tmpStr,FILTER_SEPARATOR, 0x0000 ) : 1;
	if (tmpStr) HeapFree(heap, NULL, tmpStr);
	return retCode;
}
#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS MLWatchFilter
START_DISPATCH;
CB(API_WATCHFILTER_ADDSTRING, AddString)
CB(API_WATCHFILTER_CHECK, Check)
CB(API_WATCHFILTER_COMBINE, Combine)
CB(API_WATCHFILTER_ISEMTPY, IsEmpty)
VCB(API_WATCHFILTER_CLEAR, Clear)
CB(API_WATCHFILTER_COPYTO, CopyTo)
CB(API_WATCHFILTER_GETSTRING, GetString)
CB(API_WATCHFILTER_GETSTRINGLENGTH, GetStringLength)
END_DISPATCH;
#undef CBCLASS