#include "strutil.h"
#include <windows.h>
#include <shlwapi.h>

// a function to determine the extension of a filename
// returns a pointer into the string that was passed
char *extension(const char *fn)
{
	char *x = PathFindExtensionA(fn);

	if (*x)
		return CharNextA(x);
	else
		return x;

}

const char *extensionc(const char *fn)
{
	return extension(fn);
}
