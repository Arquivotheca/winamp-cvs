#pragma once
#ifdef _WIN32
#include <windows.h>
class AutoNormalize
{
public:
	AutoNormalize()
	{
		normalized_string=0;
		size=0;
	};
	const wchar_t *Normalize(NORM_FORM form, const wchar_t *strInput)
	{
#if 1
		return strInput;
#else
		const int maxIterations = 10;

		int iSizeEstimated = NormalizeString(form, strInput, -1, NULL, 0);
		for (int i = 0; i < maxIterations; i++)
		{
			if ((size_t)iSizeEstimated > size)
			{
				free(normalized_string);
				normalized_string = (LPWSTR)malloc(iSizeEstimated * sizeof (WCHAR));
				if (!normalized_string)
				{
					size=0;
					return 0;
				}
				size = (size_t)iSizeEstimated;
			}

			iSizeEstimated = NormalizeString(form, strInput, -1, normalized_string, iSizeEstimated);

			if (iSizeEstimated > 0)
				break; // success 

			if (iSizeEstimated <= 0)
			{
				DWORD dwError = GetLastError();
				if (dwError != ERROR_INSUFFICIENT_BUFFER)  // Real error, not buffer error 
				{
					return 0;
				} 

				// New guess is negative of the return value. 
				iSizeEstimated = -iSizeEstimated;
			}
		}

		return normalized_string;
#endif
	}
protected:
	wchar_t *normalized_string;
	size_t size;
};
#endif