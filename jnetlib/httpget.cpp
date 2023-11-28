/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: httpget.cpp - JNL HTTP GET implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "httpget.h"
#ifdef USE_SSL
#include "sslconnection.h"
#endif
#include <stdio.h>
#pragma intrinsic(memcpy)
#include "../nu/strsafe.h"

JNL_HTTPGet::JNL_HTTPGet(api_dns *dns, int recvbufsize, const char *proxy)
{
	JNL::open_socketlib();
	persistent=false;
	reference_count=1;
	accept_all_reply_codes=false;
	zlibStream=0;
	allowCompression=false;
	m_dns = API_DNS_AUTODNS;
	m_con = NULL;
	m_http_proxylpinfo = 0;
	m_http_proxyhost = 0;
	m_http_proxyport = 0;
	m_sendbufsize=0;
	m_sendheaders = NULL;	
	reinit();
	open(dns, recvbufsize, proxy);
}

JNL_HTTPGet::~JNL_HTTPGet()
{
	if (zlibStream)
	{
		inflateEnd(zlibStream);
	}
	free(zlibStream);
	deinit();
	free(m_sendheaders);
	free(m_http_proxylpinfo);
	free(m_http_proxyhost);
	JNL::close_socketlib();
}

size_t JNL_HTTPGet::AddRef()
{
	return (size_t)InterlockedIncrement(&reference_count);
}

size_t JNL_HTTPGet::Release()
{
	LONG r = InterlockedDecrement(&reference_count);
	if (r == 0)
		delete this;
	return r;
}		

void JNL_HTTPGet::open(api_dns *dns, int recvbufsize, const char *proxy)
{
	m_recvbufsize = recvbufsize;
	if (dns != API_DNS_AUTODNS)
		m_dns = dns;
	if (proxy)
	{
		char *p = _strdup(proxy);
		if (p)
		{
			char *r = NULL;
			do_parse_url(p, &m_http_proxyhost, &m_http_proxyport, &r, &m_http_proxylpinfo);
			free(r);
			free(p);
		}
	}
}

void JNL_HTTPGet::reinit()
{
	m_errstr = 0;
	m_recvheaders = NULL;
	m_recvheaders_size = 0;
	m_http_state = 0;
	m_http_port = 0;
	m_http_url = 0;
	m_reply = 0;
	m_http_host = m_http_lpinfo = m_http_request = NULL;
}

void JNL_HTTPGet::deinit(bool full)
{
	if (!persistent 
		|| full
		|| (m_con && m_con->get_state() == JNL_Connection::STATE_ERROR))
	{
		delete m_con; 
		m_con = NULL;
	}
	
	free(m_recvheaders);

	free(m_http_url);
	free(m_http_host);
	free(m_http_lpinfo);
	free(m_http_request);
	free(m_errstr);
	free(m_reply);
	reinit();
}

void JNL_HTTPGet::set_sendbufsize(int sendbufsize)
{
	m_sendbufsize = sendbufsize;
}

void JNL_HTTPGet::addheader(const char *header)
{
	if (strstr(header, "\r") || strstr(header, "\n")) return ;
	if (!m_sendheaders)
	{
		size_t len = strlen(header) + 3;
		m_sendheaders = (char*)malloc(len);
		if (m_sendheaders)
		{
			char *itr=m_sendheaders;
			StringCchCopyExA(itr, len, header, &itr, &len, 0);
			StringCchCatExA(itr, len, "\r\n", &itr, &len, 0);
		}
	}
	else
	{
		size_t len = strlen(header) + strlen(m_sendheaders) + 1 + 2;
		char *t = (char*)malloc(len);
		if (t)
		{
			char *newHeaders = t;
			StringCchCopyExA(t, len, m_sendheaders, &t, &len, 0);
			StringCchCatExA(t, len, header, &t, &len, 0);
			StringCchCatExA(t, len, "\r\n", &t, &len, 0);
			free(m_sendheaders);
			m_sendheaders = newHeaders;

		}
	}
}

void JNL_HTTPGet::addheadervalue(const char *header, const char *value)
{
	size_t additional = strlen(header) + 2 + strlen(value) + 2 + 1 ;
	if (!m_sendheaders)
	{
		m_sendheaders = (char*)malloc(additional);
		if (m_sendheaders)
			StringCchPrintf(m_sendheaders, additional, "%s: %s\r\n", header, value);

	}
	else
	{
		size_t alloc_len = strlen(m_sendheaders) + additional;
		char *t = (char*)malloc(alloc_len);
		if (t)
		{
			StringCchPrintf(t, alloc_len, "%s%s: %s\r\n", m_sendheaders, header, value);
			free(m_sendheaders);
			m_sendheaders = t;
		}
	}
}

void JNL_HTTPGet::do_encode_mimestr(char *in, char *out)
{
	char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int shift = 0;
	int accum = 0;

	while (*in)
	{
		if (*in)
		{
			accum <<= 8;
			shift += 8;
			accum |= *in++;
		}
		while (shift >= 6)
		{
			shift -= 6;
			*out++ = alphabet[(accum >> shift) & 0x3F];
		}
	}
	if (shift == 4)
	{
		*out++ = alphabet[(accum & 0xF) << 2];
		*out++ = '=';
	}
	else if (shift == 2)
	{
		*out++ = alphabet[(accum & 0x3) << 4];
		*out++ = '=';
		*out++ = '=';
	}

	*out++ = 0;
}


void JNL_HTTPGet::connect(const char *url, int ver, const char *requestmethod)
{
	deinit(false);
	m_http_url = _strdup(url);
	do_parse_url(m_http_url, &m_http_host, &m_http_port, &m_http_request, &m_http_lpinfo);

	// Review 1.45 (DrO): This one cutted off the last char, so files ended with .mp instead of .mp3
	// lstrcpyn(m_http_url, url, lstrlen(url));
	// Review 1.46 (Martin): 
	// lstrlen returns the length of our char not including the terminating null character,
	// whereas lstrcpyn needs the buffersize within the termination null character, thus +1
	lstrcpyn(m_http_url, url, lstrlen(url)+1);

	if (!m_http_host || !m_http_host[0] || !m_http_port)
	{
		m_http_state = -1;
		seterrstr("invalid URL");
		return ;
	}

	size_t sendbufferlen = 0;

	if (!m_http_proxyhost || !m_http_proxyhost[0])
	{
		sendbufferlen += strlen(requestmethod) + 1 /* GET */ + strlen(m_http_request) + 9 /* HTTP/1.0 */ + 2;
	}
	else
	{
		sendbufferlen += strlen(requestmethod) + 1 /* GET */ + strlen(m_http_url) + 9 /* HTTP/1.0 */ + 2;
		if (m_http_proxylpinfo && m_http_proxylpinfo[0])
		{
			sendbufferlen += 58 + strlen(m_http_proxylpinfo) * 2; // being safe here
		}
	}
	sendbufferlen += 5 /* Host: */ + strlen(m_http_host) + 2;
	if (m_http_port != 80)
		sendbufferlen += 6;

	if (m_http_lpinfo && m_http_lpinfo[0])
	{
		sendbufferlen += 46 + strlen(m_http_lpinfo) * 2; // being safe here
	}

	if (m_sendheaders) sendbufferlen += strlen(m_sendheaders);

	size_t strLen =sendbufferlen + 1024;
	char *str = (char*)malloc(strLen);
	char *connectString = str;
	if (!str)
	{
		seterrstr("error allocating memory");
		m_http_state = -1;
	}

	if (!m_http_proxyhost || !m_http_proxyhost[0])
	{
		StringCchPrintfExA(str,strLen, &str, &strLen, 0, "%s %s HTTP/1.%d\r\n", requestmethod, m_http_request, ver % 10);
	}
	else
	{
		char *myp = NULL;
		if (strncasecmp(m_http_url, "uvox://", 7) == 0)
		{
			myp = m_http_url + 7;
			StringCchPrintfExA(str, strLen, &str, &strLen, 0, "%s http://%s HTTP/1.%d\r\n", requestmethod, myp, ver % 10);
		}
		else if (strncasecmp(m_http_url, "unsv://", 7) == 0)
		{
			myp = m_http_url + 7;
			StringCchPrintfExA(str, strLen, &str, &strLen, 0, "%s http://%s HTTP/1.%d\r\n", requestmethod, myp, ver % 10);
		}
		else if (strncasecmp(m_http_url, "uasf://", 7) == 0)
		{
			myp = m_http_url + 7;
			StringCchPrintfExA(str, strLen, &str, &strLen, 0, "%s http://%s HTTP/1.%d\r\n", requestmethod, myp, ver % 10);
		}
		else
			StringCchPrintfExA(str, strLen, &str, &strLen, 0, "%s %s HTTP/1.%d\r\n", requestmethod, m_http_url, ver % 10);
	}

	if (m_http_port == 80)
		StringCchPrintfExA(str,strLen, &str, &strLen, 0, "Host:%s\r\n", m_http_host);
	else
		StringCchPrintfExA(str,strLen, &str, &strLen, 0, "Host:%s:%d\r\n", m_http_host, m_http_port);

	if (m_http_lpinfo && m_http_lpinfo[0])
	{
		StringCchCatExA(str, strLen, "Authorization: Basic ", &str, &strLen, 0);
		do_encode_mimestr(m_http_lpinfo, str);
		StringCchCatExA(str, strLen, "\r\n", &str, &strLen, 0);
	}
	if (m_http_proxylpinfo && m_http_proxylpinfo[0])
	{
		StringCchCatExA(str, strLen, "Proxy-Authorization: Basic ", &str, &strLen, 0);
		do_encode_mimestr(m_http_proxylpinfo, str);
		StringCchCatExA(str, strLen, "\r\n", &str, &strLen, 0);
	}
	if (allowCompression)
		StringCchCatExA(str, strLen, "Accept-Encoding: gzip\r\n", &str, &strLen, 0);

	if (m_sendheaders)
		StringCchCatExA(str, strLen, m_sendheaders, &str, &strLen, 0);
	StringCchCatExA(str, strLen, "\r\n", &str, &strLen, 0);

	int a = m_recvbufsize;
	if (a < 4096) a = 4096;


	if (!m_con)
	{
	//m_con=new JNL_Connection(m_dns,strlen(str)+4,a);
	/*
	**  Joshua Teitelbaum delta 1/15/2006
	*/
#ifdef USE_SSL
	/*
	**  Joshua Teitelbaum 1/27/2006
	**  Check for secure
	*/
	if (!_strnicmp(m_http_url,"https:",strlen("https:")))
	{
		int send_buffer_size = strlen(connectString) + 4;
		send_buffer_size = min(send_buffer_size, 8192);
		send_buffer_size+=m_sendbufsize;

		if (m_sendbufsize == 0 && !strcmp(requestmethod, "POST"))
			send_buffer_size += 8192; // some extra room for posting data if it wasn't explicitly defined

		m_con=new JNL_SSL_Connection(NULL,m_dns,8192,a);
	}
	else
	{
#endif
		int send_buffer_size = strlen(connectString) + 4 + m_sendbufsize;
		if (m_sendbufsize == 0 && !strcmp(requestmethod, "POST"))
			send_buffer_size += 8192; // some extra room for posting data if it wasn't explicitly defined
		m_con=new JNL_Connection(m_dns, send_buffer_size ,a);
#ifdef USE_SSL
	}
#endif
	if (m_con)
	{
		if (!m_http_proxyhost || !m_http_proxyhost[0])
		{
			m_con->connect(m_http_host, m_http_port);
		}
		else
		{
			m_con->connect(m_http_proxyhost, m_http_proxyport);
		}
		m_con->send_string(connectString);
	}
	else
	{
		m_http_state = -1;
		seterrstr("could not create connection object");
	}
	}
	else
	{
		m_con->reuse();
		m_con->send_string(connectString);
	}
	free(connectString);
}

void JNL_HTTPGet::do_parse_url(char *url, char **host, unsigned short*port, char **req, char **lp)
{
	char *prot=0;
	
	JNL::parse_url(url, &prot, host, port, req, lp);
	if (!*port)
	{
		if (prot)
		{
			addrinfo *res;
			addrinfo hints;
			memset(&hints,0,sizeof(hints));
			hints.ai_family = PF_UNSPEC;
			hints.ai_flags = 0;
			hints.ai_socktype = SOCK_STREAM;

			if (getaddrinfo(0, prot, &hints, &res) == 0)
			{
				if (res->ai_family == AF_INET)
					*port = htons(((sockaddr_in  *)res->ai_addr)->sin_port);
				else if (res->ai_family == AF_INET6)
					*port = htons(((sockaddr_in6  *)res->ai_addr)->sin6_port);
				else // wtf?
					*port = 80;
			}
			else
				*port = 80;
		}
		else
			*port=80;
	}
	
	free(prot);

	if (!*req)
		*req = _strdup("/");
}




char *JNL_HTTPGet::getallheaders()
{
	// double null terminated, null delimited list
	if (m_recvheaders) return m_recvheaders;
	else return "\0\0";
}

char *JNL_HTTPGet::getheader(char *headername)
{
	char *ret = NULL;
	if (headername[0]==0 || !m_recvheaders) return NULL;
	size_t headername_size = strlen(headername);
	char *buf = (char*)malloc(headername_size + 2);
	lstrcpyn(buf, headername, headername_size + 2);
	if (buf[headername_size - 1] != ':') 
	{
		buf[headername_size++]=':';
		buf[headername_size]=0;
	}
	char *p = m_recvheaders;
	while (*p)
	{
		if (!strncasecmp(buf, p, headername_size))
		{
			ret = p + headername_size;
			while (*ret == ' ') ret++;
			break;
		}
		p += strlen(p) + 1;
	}
	free(buf);
	return ret;
}

int JNL_HTTPGet::run()
{
	int cnt = 0;
	if (m_http_state == -1 || !m_con)
		return HTTPRECEIVER_RUN_ERROR; // error


run_again:
	m_con->run();

	if (m_con->get_state() == JNL_Connection::STATE_ERROR)
	{
		seterrstr(m_con->get_errstr());
		return HTTPRECEIVER_RUN_ERROR;
	}
	if (m_con->get_state() == JNL_Connection::STATE_CLOSED) return HTTPRECEIVER_RUN_CONNECTION_CLOSED;

	if (m_http_state == 0) // connected, waiting for reply
	{
		if (m_con->recv_lines_available() > 0)
		{
			char buf[4096];
			m_con->recv_line(buf, 4096);
			buf[4095] = 0;
			if (m_reply && getreplycode() == 100)
			{
				free(m_reply);
				m_reply = 0;
				goto run_again;
			}	

			m_reply = _strdup(buf);
			int code = getreplycode();
			if (code >= 200 && code <= 206) 
			{
				m_http_state = 2; // proceed to read headers normally
			}
			else if (code == 301 || code == 302 || code == 303 || code == 307)
			{
				m_http_state = 1; // redirect city
			}
			else if (code != 100) // in case of HTTP 100 Continue code, we'll keep looping
			{
				if (accept_all_reply_codes)
				{
					m_http_state = 2; // proceed to read headers normally
				}
				else
				{
					seterrstr(buf);
					m_http_state = -1;
					return HTTPRECEIVER_RUN_ERROR;
				}
			}
			cnt = 0;
		}
		else if (!cnt++) goto run_again;
	}
	if (m_http_state == 1) // redirect
	{
		while (m_con->recv_lines_available() > 0)
		{
			char buf[4096];
			m_con->recv_line(buf, 4096);
			if (!buf[0])
			{
				m_http_state = -1;
				return HTTPRECEIVER_RUN_ERROR;
			}
			if (!strncasecmp(buf, "Location:", 9))
			{
				char *p = buf + 9; while (*p == ' ') p++;
				if (*p)
				{
					connect(p);
					return HTTPRECEIVER_RUN_OK;
				}
			}
		}
	}
	/* ----- read headers ----- */
	if (m_http_state == 2)
	{
		if (!cnt++ && m_con->recv_lines_available() < 1) goto run_again;
		while (m_con->recv_lines_available() > 0)
		{
			char buf[8192];
			m_con->recv_line(buf, 8192);
			if (!buf[0])
			{
				const char *compression = getheader("Content-Encoding");
				if (compression && !strcmp(compression, "gzip"))
				{
					zlibStream = (z_stream *)malloc(sizeof(z_stream));
					zlibStream->next_in = Z_NULL;
					zlibStream->avail_in = Z_NULL;
					zlibStream->next_out = Z_NULL;
					zlibStream->avail_out = Z_NULL;
					zlibStream->zalloc = (alloc_func)0;
					zlibStream->zfree = (free_func)0;
					zlibStream->opaque = 0;
					int z_err = inflateInit2(zlibStream, 15 + 16 /* +16 for gzip */);
					if (z_err != Z_OK)
					{
						free(zlibStream);
						zlibStream=0;
					}
				}
				m_http_state = 3;
				break;
			}
			if (!m_recvheaders)
			{
				m_recvheaders_size = strlen(buf) + 1;
				if (m_recvheaders_size == 0 || m_recvheaders_size == (size_t)-1) // check for overflow
				{
					m_http_state = -1;
					return HTTPRECEIVER_RUN_ERROR;
				}
				m_recvheaders = (char*)malloc(m_recvheaders_size + 1);
				if (m_recvheaders)
				{
					strcpy(m_recvheaders, buf); // safe because we malloc'd specifically above
					m_recvheaders[m_recvheaders_size] = 0;
				}
				else
				{
					m_http_state = -1;
					return HTTPRECEIVER_RUN_ERROR;
				}
			}
			else
			{
				size_t oldsize = m_recvheaders_size;
				m_recvheaders_size += strlen(buf) + 1;
				if (m_recvheaders_size+1 < oldsize) // check for overflow
				{
					m_http_state = -1;
					return HTTPRECEIVER_RUN_ERROR;
				}
				char *n = (char*)realloc(m_recvheaders, m_recvheaders_size + 1);
				if (!n)
				{
					m_http_state = -1;
					return HTTPRECEIVER_RUN_ERROR;
				}
				strcpy(n+oldsize, buf); // safe because we malloc specifially for the size
				n[m_recvheaders_size] = 0; // double null terminate
				m_recvheaders = n;
			}
		}
	}
	if (m_http_state == 3)
	{

	}

	return HTTPRECEIVER_RUN_OK;
}

int JNL_HTTPGet::get_status() // returns 0 if connecting, 1 if reading headers,
// 2 if reading content, -1 if error.
{
	if (m_http_state < 0) return HTTPRECEIVER_STATUS_ERROR;
	if (m_http_state < 2) return HTTPRECEIVER_STATUS_CONNECTING;
	if (m_http_state == 2) return HTTPRECEIVER_STATUS_READING_HEADERS;
	if (m_http_state == 3) return HTTPRECEIVER_STATUS_READING_CONTENT;
	return HTTPRECEIVER_STATUS_ERROR;
}

int JNL_HTTPGet::getreplycode() // returns 0 if none yet, otherwise returns http reply code.
{
	if (!m_reply) return 0;
	char *p = m_reply;
	while (*p && *p != ' ') p++; // skip over HTTP/x.x
	if (!*p) return 0;
	return atoi(++p);
}

int JNL_HTTPGet::bytes_available()
{
	if (m_con && m_http_state == 3)
		return m_con->recv_bytes_available();
	return 0;
}
int JNL_HTTPGet::get_bytes(char *buf, int len)
{
	if (m_con && m_http_state == 3)
	{
		if (zlibStream)
		{
			// TODO: benski> we need to pick a better buffer size
			// either alloca() and use the passed in length
			// or malloc a buffer based on the constructor-initted buffer size
			char temp[8192];
			int size=m_con->peek_bytes(temp, 8192);
			if (size)
			{
				zlibStream->next_in = reinterpret_cast<Bytef*>(temp);
				zlibStream->avail_in = (uInt)size;
				zlibStream->next_out = reinterpret_cast<Bytef*>(buf);
				zlibStream->avail_out = len;
				int zlib_err = inflate(zlibStream, Z_SYNC_FLUSH);
				//zlib_err = inflate(stream, Z_NO_FLUSH);
				if (zlib_err == Z_OK || zlib_err == Z_STREAM_END)
				{
					m_con->recv_bytes(0, size-zlibStream->avail_in); // since we only peeked above
					return len-zlibStream->avail_out;
				}
				else
					return 0; // TODO: should we do something else here?
			}
		}
		else
			return m_con->recv_bytes(buf, len);
	}
	return 0;
}
int JNL_HTTPGet::peek_bytes(char *buf, int len)
{

	if (m_con && m_http_state == 3)
	{
		if (zlibStream)
		{
			return 0; // TODO: benski> how are we going to do peek_bytes, since the inflater saves state?
		}
		else

			return m_con->peek_bytes(buf, len);
	}
	return 0;
}

int JNL_HTTPGet::content_length()
{
	const char *p = getheader("content-length");
	if (p)
		return atoi(p);
	return 0;
}

void JNL_HTTPGet::seterrstr(char *str)
{
	free(m_errstr);
	m_errstr =_strdup(str);
}

void JNL_HTTPGet::AllowCompression()
{
	allowCompression=true;
}

void JNL_HTTPGet::reset_headers()
{
	if (m_sendheaders)
	{
		free(m_sendheaders);
		m_sendheaders=0;
	}
}

void JNL_HTTPGet::set_accept_all_reply_codes()
{
	accept_all_reply_codes=true;
}

void JNL_HTTPGet::set_persistent()
{
	persistent=true;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS JNL_HTTPGet
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
VCB(API_HTTPRECEIVER_OPEN, open)
VCB(API_HTTPRECEIVER_ADDHEADER, addheader)
VCB(API_HTTPRECEIVER_ADDHEADERVALUE, addheadervalue)
VCB(API_HTTPRECEIVER_CONNECT, connect)
CB(API_HTTPRECEIVER_RUN, run)
CB(API_HTTPRECEIVER_GETSTATUS, get_status)
CB(API_HTTPRECEIVER_GETBYTESAVAILABLE, bytes_available)
CB(API_HTTPRECEIVER_GETBYTES, get_bytes)
CB(API_HTTPRECEIVER_PEEKBYTES, peek_bytes)
CB(API_HTTPRECEIVER_GETHEADER, getheader)
CB(API_HTTPRECEIVER_GETCONTENTLENGTH, content_length)
CB(API_HTTPRECEIVER_GETALLHEADERS, getallheaders)
CB(API_HTTPRECEIVER_GETREPLYCODE, getreplycode)
CB(API_HTTPRECEIVER_GETREPLY, getreply)
CB(API_HTTPRECEIVER_GETERROR, geterrorstr)
CB(API_HTTPRECEIVER_GETCONNECTION, get_con)
VCB(API_HTTPRECEIVER_ALLOW_COMPRESSION, AllowCompression)
VCB(API_HTTPRECEIVER_RESET_HEADERS, reset_headers)
CB(API_HTTPRECEIVER_GET_URL, get_url)
VCB(API_HTTPRECEIVER_SET_SENDBUFSIZE, set_sendbufsize)
VCB(API_HTTPRECEIVER_SET_ACCEPT_ALL_REPLY_CODES, set_accept_all_reply_codes)
VCB(API_HTTPRECEIVER_SET_PERSISTENT, set_persistent)
END_DISPATCH;
#undef CBCLASS
