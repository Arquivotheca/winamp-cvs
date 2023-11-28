#pragma once
#include "foundation/types.h"
#include "foundation/error.h"
#include "sqlite/sqlite3.h"
#include "nx/nxstring.h"
#include <assert.h>

#ifndef __APPLE__
static void sqlite3_nxstring_release(void *data)
{
	if (data)
	{
		nx_string_t self = (nx_string_t )(((int8_t *)data)-2*sizeof(size_t));
		NXStringRelease(self);
	}
}

static void sqlite3_nxuri_release(void *data)
{
	if (data)
	{
		nx_uri_t self = (nx_uri_t )(((int8_t *)data)-2*sizeof(size_t));
		NXURIRelease(self);
	}
}
#else
static void sqlite3_nxstring_release(void *data)
{
	free(data);
}

static void sqlite3_nxuri_release(void *data)
{
	free(data);
}
#endif

/* ------------------------------------------------------------------------ */
static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, int value)
{
	return sqlite3_bind_int(statement, position, value);
}

static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, int64_t value)
{
	return sqlite3_bind_int64(statement, position, value);
}

static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, double value)
{
	return sqlite3_bind_double(statement, position, value);
}

static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, const char *value)
{
	if (value)
		return sqlite3_bind_text(statement, position, value, -1, SQLITE_TRANSIENT);
	else
		return sqlite3_bind_null(statement, position);
}

static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, nx_string_t value)
{
#ifdef _WIN32
	if (value)
	{
		NXStringRetain(value);
		assert(wcslen(value->string) == value->len);
		return sqlite3_bind_text16(statement, position, value->string, value->len*sizeof(wchar_t), sqlite3_nxstring_release);
	}
	else
		return sqlite3_bind_null(statement, position);
#elif defined(__APPLE__)
	if (value)
	{
		size_t length = NXStringGetLength(value);
		size_t bufferSize = (length + 1)  * sizeof(UniChar);
		UniChar *buffer = (UniChar *)malloc(bufferSize);
		NXStringGetCharacters(value, 0, length,  buffer);
		return sqlite3_bind_text16(statement, position, buffer, (int)(bufferSize - sizeof(UniChar)), sqlite3_nxstring_release);
	}
	else
		return sqlite3_bind_null(statement, position);
#else
	if (value)
	{
		//NXStringRetain(value);
		return sqlite3_bind_text(statement, position, value->string, value->len, SQLITE_TRANSIENT);
	}
	else
		return sqlite3_bind_null(statement, position);
#endif
}

static inline int sqlite3_bind_any(sqlite3_stmt *statement, int position, nx_uri_t value)
{
#ifdef _WIN32
	if (value)
	{
		//NXURIRetain(value);
		assert(wcslen(value->string) == value->len);
		return sqlite3_bind_text16(statement, position, value->string, value->len*sizeof(wchar_t), SQLITE_TRANSIENT);
	}
	else
		return sqlite3_bind_null(statement, position);
#elif defined(__APPLE__)
	if (value)
	{
		nx_string_t string;
		NXURIGetNXString(&string, value);
		size_t length = NXStringGetLength(string);
		size_t bufferSize = (length + 1)  * sizeof(UniChar);
		UniChar *buffer = (UniChar *)malloc(bufferSize);
		NXStringGetCharacters(string, 0, length,  buffer);
		NXStringRelease(string);
		return sqlite3_bind_text16(statement, position, buffer, (int)(bufferSize - sizeof(UniChar)), sqlite3_nxstring_release);
	}
	else
		return sqlite3_bind_null(statement, position);

#else
	if (value)
	{
		//NXURIRetain(value);
		return sqlite3_bind_text(statement, position, value->string, value->len, SQLITE_TRANSIENT);
	}
	else
		return sqlite3_bind_null(statement, position);
#endif
}


/* ------------------------------------------------------------------------ */
static inline void sqlite3_column_any(sqlite3_stmt *statement, int position, int *value)
{
	*value = sqlite3_column_int(statement, position);
}

static inline void sqlite3_column_any(sqlite3_stmt *statement, int position, int64_t *value)
{
	*value = sqlite3_column_int64(statement, position);
}

static inline void sqlite3_column_any(sqlite3_stmt *statement, int position, nx_string_t *value)
{
	const unsigned char *utf8 = sqlite3_column_text(statement, position);
	NXStringCreateWithUTF8(value, (const char *)utf8);		
}

static inline void sqlite3_column_any(sqlite3_stmt *statement, int position, nx_uri_t *value)
{
	const unsigned char *utf8 = sqlite3_column_text(statement, position);
	NXURICreateWithUTF8(value, (const char *)utf8);		
}

static inline void sqlite3_column_any(sqlite3_stmt *statement, int position, double *value)
{
	*value = sqlite3_column_double(statement, position);
}