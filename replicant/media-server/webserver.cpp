/*
** JNetLib
** Copyright (C) 2000-2003 Nullsoft, Inc.
** Author: Justin Frankel
** File: webserver.cpp - Generic simple webserver baseclass
** License: see jnetlib.h
** see test.cpp for an example of how to use this class
*/

#include "webserver.h"
#include "foundation/error.h"
#include <time.h>
#include <stdlib.h>

#define PACKET_SIZE 8192

ConnectionInstance::ConnectionInstance(jnl_connection_t c)
{
	connection = c;
	m_pagegen = 0;
	time(&m_connect_time);
	jnl_http_request_create(&m_serv, c);
}

ConnectionInstance::~ConnectionInstance()
{
	if (m_pagegen)
	{
		m_pagegen->Release();
	}
	jnl_http_request_release(m_serv);
}

void ConnectionInstance::close()
{
	if (m_pagegen)
	{
		m_pagegen->Release();
		m_pagegen=0;
	}
	time(&m_connect_time);
}

WebServer::WebServer()
{
	m_listener_rot = 0;
	m_timeout_s = 15;
	m_max_con = 100;
}

WebServer::~WebServer()
{
	m_connections.deleteAll();
	
	for (size_t x = 0; x < m_listeners.size(); x ++)
	{
		jnl_listen_release(m_listeners[x]);
	}
}

void WebServer::SetMaxConnections(size_t max_con)
{
	m_max_con = max_con;
}

void WebServer::SetRequestTimeout(time_t timeout_s)
{
	m_timeout_s = timeout_s;
}

int WebServer::addListenPort(unsigned short port)
{
	removeListenPort(port);

	struct addrinfo *addr;
	if (jnl_dns_resolve_now(0, port, &addr, SOCK_STREAM) == NErr_Success)
	{
		size_t index=0;
		jnl_listen_t p;
		while (jnl_listen_create_from_address(&p, addr, index++) == NErr_Success)
		{
			m_listeners.push_back(p);
		}
	}
	
	return NErr_Success;
}

void WebServer::removeListenPort(unsigned short  port)
{
	for (size_t x = 0; x < m_listeners.size(); x ++)
	{
		jnl_listen_t p = m_listeners[x];

		if (jnl_listen_get_port(p) == port)
		{
			jnl_listen_release(p);
			m_listeners.eraseindex(x);
			break;
		}

	}
}

void WebServer::removeListenIdx(size_t idx)
{
	if (idx >= 0 && idx < m_listeners.size())
	{
		jnl_listen_release(m_listeners[idx]);
		m_listeners.eraseindex(idx);
	}
}

unsigned short WebServer::getListenPort(size_t idx, int *err)
{
	jnl_listen_t p = m_listeners[idx];
	if (p)
	{
		//if (err) *err = p->is_error();
		return jnl_listen_get_port(p);
	}

	return 0;
}


void WebServer::run(void)
{
	size_t nl;
	if (m_connections.size() < m_max_con && (nl = m_listeners.size()))
	{
		jnl_listen_t l = m_listeners.at(m_listener_rot++ % nl);
		
		jnl_connection_t c = jnl_listen_get_connection(l);
		if (c)
		{
			add_connection(c, l);
		}
	}
	size_t x;
	for (x = 0; x < m_connections.size(); x ++)
	{
		ConnectionInstance *this_con = m_connections.at(x);
		switch(run_connection(this_con))
		{
		case RUN_CONNECTION_DONE:
			{
				bool keep_alive = false;
				if (jnl_htt_request_get_keep_alive(this_con->m_serv))
				{
					const char *connection_status = jnl_http_request_get_header(this_con->m_serv, "Connection");
					if (!connection_status || _stricmp(connection_status, "close"))
						keep_alive=true;
				}
				if (!keep_alive)
				{
					delete this_con;
					m_connections.eraseindex(x--);
				}
				else
				{
					jnl_http_request_reset(this_con->m_serv);
				}
			}
			break;
		case RUN_CONNECTION_ERROR:
		case RUN_CONNECTION_TIMEOUT:
			delete this_con;
			m_connections.eraseindex(x--);
			break;
		}
	}
}
void WebServer::SetConnectionCallback(ifc_connection_callback *_connectionCallback)
{
	connectionCallback = _connectionCallback;
}

void WebServer::add_connection(jnl_connection_t con, jnl_listen_t listener)
{
	m_connections.push_back(new ConnectionInstance(con));
}

int WebServer::run_connection(ConnectionInstance *con)
{
	// TODO: add a Run() method to WC_conInst, passing in connectionCallback
	int s = jnl_http_request_run(con->m_serv);
	if (s < 0)
	{
		// m_serv.geterrorstr()
		return RUN_CONNECTION_ERROR;
	}
	if (s < 2)
	{
		// return 1 if we timed out
		if (time(NULL) - con->m_connect_time > m_timeout_s)
			return RUN_CONNECTION_TIMEOUT;
		else
			return RUN_CONNECTION_CONTINUE;
	}
	if (s < 3)
	{
		con->m_pagegen = connectionCallback->OnConnection(con->m_serv);

		return RUN_CONNECTION_CONTINUE;
	}
	if (s < 4)
	{
		if (!con->m_pagegen)
		{
			if (jnl_connection_send_bytes_in_queue(con->connection) == 0)
					return RUN_CONNECTION_DONE;
				else
					return RUN_CONNECTION_CONTINUE;
		}
		char buf[PACKET_SIZE];
		size_t l = jnl_connection_send_bytes_available(con->connection);
		if (l > 0)
		{
			if (l > sizeof(buf))
				l = sizeof(buf);
			l = con->m_pagegen->GetData(buf, l);
			if (l < (con->m_pagegen->IsNonBlocking() ? 0 : 1)) // if nonblocking, this is l < 0, otherwise it's l<1
			{
				if (jnl_connection_send_bytes_in_queue(con->connection) == 0)
					return RUN_CONNECTION_DONE;
				else
					return RUN_CONNECTION_CONTINUE;
			}
			if (l > 0)
			{
				jnl_connection_send(con->connection, buf, l);
				
			}
		}
		return RUN_CONNECTION_CONTINUE;
	}
	return RUN_CONNECTION_DONE; // we're done by this point
}

void WebServer::url_encode(char *in, char *out, int max_out)
{
	while (*in && max_out > 4)
	{
		if ((*in >= 'A' && *in <= 'Z') ||
		        (*in >= 'a' && *in <= 'z') ||
		        (*in >= '0' && *in <= '9') || *in == '.' || *in == '_' || *in == '-')
		{
			*out++ = *in++;
			max_out--;
		}
		else
		{
			int i = *in++;
			*out++ = '%';
			int b = (i >> 4) & 15;
			if (b < 10) *out++ = '0' + b;
			else *out++ = 'A' + b - 10;
			b = i & 15;
			if (b < 10) *out++ = '0' + b;
			else *out++ = 'A' + b - 10;
			max_out -= 3;
		}
	}
	*out = 0;
}

void WebServer::url_decode(char *in, char *out, int maxlen)
{
	while (*in && maxlen > 1)
	{
		if (*in == '+')
		{
			in++;
			*out++ = ' ';
		}
		else if (*in == '%' && in[1] != '%' && in[1])
		{
			int a = 0;
			int b = 0;
			for ( b = 0; b < 2; b ++)
			{
				int r = in[1 + b];
				if (r >= '0' && r <= '9') r -= '0';
				else if (r >= 'a' && r <= 'z') r -= 'a' -10;
				else if (r >= 'A' && r <= 'Z') r -= 'A' -10;
				else break;
				a *= 16;
				a += r;
			}
			if (b < 2) *out++ = *in++;
		else { *out++ = a; in += 3;}
		}
		else *out++ = *in++;
		maxlen--;
	}
	*out = 0;
}

void WebServer::base64decode(char *src, char *dest, int destsize)
{
	int accum = 0;
	int nbits = 0;
	while (*src)
	{
		int x = 0;
		char c = *src++;
		if (c >= 'A' && c <= 'Z') x = c - 'A';
		else if (c >= 'a' && c <= 'z') x = c - 'a' + 26;
		else if (c >= '0' && c <= '9') x = c - '0' + 52;
		else if (c == '+') x = 62;
		else if (c == '/') x = 63;
		else break;

		accum <<= 6;
		accum |= x;
		nbits += 6;

		while (nbits >= 8)
		{
			if (--destsize <= 0) break;
			nbits -= 8;
			*dest++ = (char)((accum >> nbits) & 0xff);
		}

	}
	*dest = 0;
}

void WebServer::base64encode(char *in, char *out)
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
		while ( shift >= 6 )
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

int WebServer::parseAuth(char *auth_header, char *out, int out_len) //returns 0 on unknown auth, 1 on basic
{
	char *authstr = auth_header;
	*out = 0;
	if (!auth_header || !*auth_header) return 0;
	while (*authstr == ' ') authstr++;
	if (_strnicmp(authstr, "basic ", 6)) return 0;
	authstr += 6;
	while (*authstr == ' ') authstr++;
	base64decode(authstr, out, out_len);
	return 1;
}
