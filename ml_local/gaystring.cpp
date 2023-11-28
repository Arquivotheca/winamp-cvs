#include "main.h"
#include "ml_local.h"
#include "gaystring.h"

GayString::GayString(const char *initial)
{
	m_buf = NULL;
	m_alloc = 0;
	if (initial) Set(initial);
}

GayString::~GayString()
{
	free(m_buf);
}

void GayString::Set(const char *value)
{
	if (!value)
		value="";
	Grow(strlen(value) + 1);
	strcpy(m_buf, value);
}

char *GayString::Get() { return m_buf ? m_buf : (char*)""; }

void GayString::Append(const char *append)
{
  size_t oldsize = m_buf ? strlen(m_buf) : 0;
  Grow(oldsize + strlen(append) + 1);
  strcpy(m_buf + oldsize, append);
}

void GayString::Grow(size_t newsize)
{
	if (m_alloc < newsize)
	{
		m_alloc = newsize + 512;
		char* m_old_buf = m_buf;
		m_buf = (char*)realloc(m_buf, m_alloc);
		if (!m_buf && m_alloc > 0) m_buf = m_old_buf;
	}
}

void GayString::Compact()
{
	if (m_buf)
	{
		m_alloc = strlen(m_buf) + 1;
		char* m_old_buf = m_buf;
		m_buf = (char*)realloc(m_buf, m_alloc);
		if (!m_buf) m_buf = m_old_buf;
	}
}

/* */
GayStringW::GayStringW(const wchar_t *initial)
{
	len=0;
	m_buf = NULL;
	m_alloc = 0;
	if (initial) Set(initial);
}

GayStringW::~GayStringW()
{
	free(m_buf);
}

void GayStringW::Set(const wchar_t *value)
{
	if (!value)
		value=L"";
	len = wcslen(value);
	Grow(len + 1);
	wcscpy(m_buf, value);
}

const wchar_t *GayStringW::Get() { return m_buf ? m_buf : L""; }

void GayStringW::Append(const wchar_t *append)
{
	size_t oldsize = len;
	len += wcslen(append);
	Grow(len + 1);
	wcscpy(m_buf + oldsize, append);
}

void GayStringW::Grow(size_t newsize)
{
	if (m_alloc < newsize)
	{
		m_alloc = newsize + 512;
		wchar_t* m_old_buf = m_buf;
		m_buf = (wchar_t*)realloc(m_buf, m_alloc * sizeof(wchar_t));
		if (!m_buf && m_alloc > 0) m_buf = m_old_buf;
	}
}

void GayStringW::Compact()
{
	if (m_buf)
	{
		m_alloc = len + 1;
		wchar_t* m_old_buf = m_buf;
		m_buf = (wchar_t *)realloc(m_buf, m_alloc * sizeof(wchar_t));
		if (!m_buf) m_buf = m_old_buf;
	}
}

size_t GayStringW::Length() { return len; }