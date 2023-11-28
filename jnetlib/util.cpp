/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: util.cpp - JNL implementation of basic network utilities
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include <bfc/error.h>

int JNL::open_socketlib()
{
#ifdef _WIN32

		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(1, 1), &wsaData)) return 1;

#endif
  return 0;
}
void JNL::close_socketlib()
{
#ifdef _WIN32

		WSACleanup();

#endif
}

static char *jnl_strndup(const char *str, size_t n)
{
	char *o = (char *)malloc(n+1);
	if (!o)
		return 0;
	
	strncpy(o, str, n);
	o[n]=0;
	return o;
}

int JNL::parse_url(const char *url, char **prot, char **host, unsigned short *port, char **req, char **lp)
{
	free(*prot); *prot=0;
	free(*host); *host = 0;
	free(*req); *req = 0;
	free(*lp); *lp = 0;
	*port = 0;

	const char *p;
	const char *protocol = strstr(url, "://");
	if (protocol)
	{
		*prot = jnl_strndup(url, protocol-url);
		p = protocol + 3;
	
	}
	else
	{
		p = url;
	}

	while (*p == '/') p++; // skip extra /

	size_t end = strcspn(p, "@/");

	// check for username
	if (p[end] == '@')
	{
		*lp = jnl_strndup(p, end);
		p = p+end+1;
		end = strcspn(p, "[:/");
	}

	if (p[0] == '[') // IPv6 style address
	{
		p++;
		const char *ipv6_end = strchr(p, ']');
		if (!ipv6_end)
			return NErr_Malformed;

		*host = jnl_strndup(p, ipv6_end-p);
		p = ipv6_end+1;
	}
	else
	{
		end = strcspn(p, ":/");
		*host = jnl_strndup(p, end);
		p += end;
	}

	// is there a port number?
	if (p[0] == ':')
	{
		char *new_end;
		*port = (unsigned short)strtoul(p+1, &new_end, 10);
		p = new_end;
	}

	if (p[0])	
	{
		// benski> this is here to workaround a bug with YP and NSV streams
		if (!strcmp(p, ";stream.nsv"))
			return NErr_Success;

		*req = _strdup(p);
	}
	
	return NErr_Success;
}


#if 0
unsigned long JNL::ipstr_to_addr(const char *cp) 
{ 
  return inet_addr(cp); 
}

void JNL::addr_to_ipstr(unsigned long addr, char *host, int maxhostlen) 
{ 
	in_addr a; a.s_addr=addr;
	sprintf(host, /*maxhostlen,*/ "%u.%u.%u.%u", a.S_un.S_un_b.s_b1, a.S_un.S_un_b.s_b2, a.S_un.S_un_b.s_b3,a.S_un.S_un_b.s_b4);
  //char *p=::inet_ntoa(a); strncpy(host,p?p:"",maxhostlen);
}
#endif