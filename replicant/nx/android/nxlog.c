#include "nxlog.h"
#include <stdio.h>
#include <stdarg.h>
#include <android/log.h>

static char *nx_log_tag = "libreplicant";

void NXLog(int priority, char *fmt, ...)
{	
	 va_list argptr;
	 va_start(argptr,fmt);
	__android_log_vprint(priority, nx_log_tag, fmt, argptr);
	va_end(argptr);
}
