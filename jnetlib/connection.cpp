/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: connection.cpp - JNL TCP connection implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "connection.h"
#include "asyncdns.h"
#include "../nu/strsafe.h"
#pragma intrinsic(memcpy, memset)

JNL_Connection::JNL_Connection()
{
	init();
}

JNL_Connection::JNL_Connection(api_dns *dns, size_t sendbufsize, size_t recvbufsize)
{
	init();
	open(dns, sendbufsize, recvbufsize);
}


void JNL_Connection::init()
{
	m_errorstr="";
	address=0;
	m_dns=0;
	m_dns_owned=false;
	m_socket=-1;
	m_remote_port=0;
	m_state=STATE_NOCONNECTION;
	m_host[0]=0;
	saddr=0;
}

JNL_Connection::~JNL_Connection()
{
	/*
	**  Joshua Teitelbaum 1/27/2006
	**  virtualization for ssl, calling socket_shtudown()
	*/
	socket_shutdown();

	if (!saddr) // free it if it was passed to us (by JNL_Listen, presumably)
		free(address); // TODO: change this if we ever do round-robin DNS connecting or in any way change how we handle 'address'

	if (m_dns_owned) 
	{
		delete static_cast<JNL_AsyncDNS *>(m_dns);
	}
}

void JNL_Connection::set_dns(api_dns *dns)
{
	if (m_dns_owned)
		delete static_cast<JNL_AsyncDNS *>(m_dns);
	m_dns=dns;
	m_dns_owned=false;
}

void JNL_Connection::open(api_dns *dns, size_t sendbufsize, size_t recvbufsize)
{
	if (dns != API_DNS_AUTODNS && dns)
	{
		m_dns=dns;
		m_dns_owned=false;
	}
	else if (!m_dns)
	{
		m_dns=new JNL_AsyncDNS;
		m_dns_owned=true;
	}

	recv_buffer.reserve(recvbufsize);
	send_buffer.reserve(sendbufsize);
}

void JNL_Connection::connect(int s, sockaddr *addr, int length)
{
	close(1);
	m_socket=s;
	address=(sockaddr *)malloc(length);
	memcpy(address, addr, length);

	m_remote_port=0;
	if (m_socket != -1)
	{
		SET_SOCK_BLOCK(m_socket,0);
		m_state=STATE_CONNECTED;
	}
	else 
	{
		m_errorstr="invalid socket passed to connect";
		m_state=STATE_ERROR;
	}

}

void JNL_Connection::connect(char *hostname, int port)
{
	close(1);
	m_remote_port=(short)port;

	StringCbCopyA(m_host, sizeof(m_host), hostname);

	//memset(&m_saddr,0,sizeof(m_saddr));
	if (!m_host[0])
	{
		m_errorstr="empty hostname";
		m_state=STATE_ERROR;
	}
	else
	{
		m_state=STATE_RESOLVING;
	}
}

/*
**  Joshua Teitelbaum 1/27/2006
**  socket_shutdown
**  virtualization for ssl
*/
/* Virtual */ 
void JNL_Connection::socket_shutdown()
{
	if (m_socket >= 0)
	{
		::shutdown(m_socket, SHUT_RDWR);
		::closesocket(m_socket);
		m_socket=-1;
	}
}
/*
**  Joshua Teitelbaum 1/27/2006
**  socket_recv
**  virtualization for ssl
*/
/* Virtual */ 
ssize_t JNL_Connection::socket_recv(char *buf, size_t len, int options)
{
	return ::recv(m_socket,buf,len,options);
}
/*
**  Joshua Teitelbaum 1/27/2006
**  socket_send
**  virtualization for ssl
*/
/* Virtual */ 
int JNL_Connection::socket_send(const char *buf, int len, int options)
{
	return ::send(m_socket,buf,len,options);
}

int JNL_Connection::socket_connect()
{
	return ::connect(m_socket, saddr->ai_addr, saddr->ai_addrlen);
	//return ::connect(m_socket,(struct sockaddr *)&m_saddr, sizeof(m_saddr));
}

void JNL_Connection::run(int max_send_bytes, int max_recv_bytes, int *bytes_sent, int *bytes_rcvd)
{
	int bytes_allowed_to_send=(max_send_bytes<0)?send_buffer.size():max_send_bytes;
	int bytes_allowed_to_recv=(max_recv_bytes<0)?recv_buffer.avail():max_recv_bytes;

	if (bytes_sent) *bytes_sent=0;
	if (bytes_rcvd) *bytes_rcvd=0;

	switch (m_state)
	{
	case STATE_RESOLVING:
		if (saddr==0)
		{
			int a=m_dns->resolve(m_host, m_remote_port, &saddr, SOCK_STREAM);
			if (!a) { m_state=STATE_RESOLVED; }
			else if (a == 1)
			{
				m_state=STATE_RESOLVING; 
				break;
			}
			else
			{
				m_errorstr="resolving hostname"; 
				m_state=STATE_ERROR; 
				return;
			}
		}
		// fall through
	case STATE_RESOLVED:
		m_socket=::socket(saddr->ai_family, saddr->ai_socktype, saddr->ai_protocol);
		if (m_socket==-1)
		{
			m_errorstr="creating socket";
			m_state=STATE_ERROR;
		}
		else
		{
			SET_SOCK_BLOCK(m_socket,0);
		}

		/*
		**  Joshua Teitelbaum 1/27/2006
		**  virtualization for ssl
		*/
		if(!socket_connect())
		{
			address=saddr->ai_addr;
			m_state=STATE_CONNECTED;
			on_socket_connected();
		}
		else if (ERRNO!=EINPROGRESS)
		{
			m_errorstr="connecting to host";
			m_state=STATE_ERROR;
		}
		else { m_state=STATE_CONNECTING; }
		break;
	case STATE_CONNECTING:
		{		
			fd_set f[3];
			FD_ZERO(&f[0]);
			FD_ZERO(&f[1]);
			FD_ZERO(&f[2]);
			FD_SET(m_socket,&f[0]);
			FD_SET(m_socket,&f[1]);
			FD_SET(m_socket,&f[2]);
			struct timeval tv;
			memset(&tv,0,sizeof(tv));
			if (select(m_socket+1,&f[0],&f[1],&f[2],&tv)==-1)
			{
				m_errorstr="connecting to host (calling select())";
				m_state=STATE_ERROR;
			}
			else if (FD_ISSET(m_socket,&f[1])) 
			{
				m_state=STATE_CONNECTED;
				on_socket_connected();
			}
			else if (FD_ISSET(m_socket,&f[2]))
			{
				m_errorstr="connecting to host";
				m_state=STATE_ERROR;
			}
		}
		break;
	case STATE_CONNECTED:
	case STATE_CLOSING:
		/* --- send --- */		
		{
			size_t sent = send_buffer.drain(this, bytes_allowed_to_send);
			if (bytes_sent)
				*bytes_sent+=sent;

			if (m_state == STATE_CLOSED)
				break;

		/* --- receive --- */
			size_t received = recv_buffer.fill(this, bytes_allowed_to_recv);
			if (bytes_rcvd)
				*bytes_rcvd+=received;
		}

		if (m_state == STATE_CLOSING)
		{
			if (send_buffer.empty()) m_state = STATE_CLOSED;
		}
		break;
	default: break;
	}
}

void JNL_Connection::on_socket_connected(void)
{
	return;
}

void JNL_Connection::close(int quick)
{
	if (quick || m_state == STATE_RESOLVING || m_state == STATE_CONNECTING)
	{
		m_state=STATE_CLOSED;
		/*
		**  Joshua Teitelbaum 1/27/2006
		**  virualization for ssl
		*/
		socket_shutdown();

		m_socket=-1;
		recv_buffer.clear();
		send_buffer.clear();
		m_remote_port=0;
		m_host[0]=0;
		//memset(&m_saddr,0,sizeof(m_saddr));
	}
	else
	{
		if (m_state == STATE_CONNECTED) m_state=STATE_CLOSING;
	}
}

size_t JNL_Connection::send_bytes_in_queue(void)
{
	return send_buffer.size();
}

size_t JNL_Connection::send_bytes_available(void)
{
	return send_buffer.avail();
}

int JNL_Connection::send(const void *data, int length)
{
	if (length > (int)send_bytes_available())
	{
		return -1;
	}

	send_buffer.write(data, length);
	return 0;
}

int JNL_Connection::send_string(const char *line)
{
	return send(line,strlen(line));
}

size_t JNL_Connection::recv_bytes_available(void)
{
	return recv_buffer.size();
}

size_t JNL_Connection::peek_bytes(void *data, size_t maxlength)
{
	if (data)
		return recv_buffer.peek(data, maxlength);
	else
		return min(maxlength, recv_bytes_available());
}

size_t JNL_Connection::recv_bytes(void *data, size_t maxlength)
{
	if (data)
		return recv_buffer.read(data, maxlength);
	else
		return recv_buffer.advance(maxlength);
}

int JNL_Connection::recv_lines_available(void)
{
	int l=recv_bytes_available();
	int lcount=0;
	int lastch=0;
	int pos;
	for (pos=0; pos < l; pos ++)
	{
		char t;
		if (recv_buffer.at(pos, &t, 1) != 1)
			return lcount;
		if ((t=='\r' || t=='\n') &&(
			(lastch != '\r' && lastch != '\n') || lastch==t
			)) lcount++;
		lastch=t;
	}
	return lcount;
}

int JNL_Connection::recv_line(char *line, int maxlength)
{
	while (maxlength--)
	{
		char t;
		if (recv_buffer.read(&t, 1) == 0) 
		{
			*line=0;
			return 0;
		}
		if (t == '\r' || t == '\n')
		{
			char r;
			if (recv_buffer.peek(&r, 1) != 0)
			{
				if ((r == '\r' || r == '\n') && r != t)
					recv_buffer.advance(1);
			}
			*line=0;
			return 0;
			
		}
		*line++=t;
	}
	return 1;
}

unsigned long JNL_Connection::get_interface(void)
{
	if (m_socket==-1) return 0;
	struct sockaddr_in sin;
	memset(&sin,0,sizeof(sin));
	socklen_t len=sizeof(sin);
	if (::getsockname(m_socket,(struct sockaddr *)&sin,&len)) return 0;
	return (unsigned long) sin.sin_addr.s_addr;
}

unsigned long JNL_Connection::get_remote()
{
	// TODO: IPv6
	if (address)
	{
		sockaddr_in *ipv4 = (sockaddr_in *)address;
		return ipv4->sin_addr.s_addr;
	}
	return 0;

}

short JNL_Connection::get_remote_port()
{
	return m_remote_port;
}

/* RingBuffer client function */
size_t JNL_Connection::Read(void *dest, size_t len)
{
	if (!len)
		return 0;
	int res=socket_recv((char *)dest,len,0);
	if (res == 0 || (res < 0 && ERRNO != EWOULDBLOCK))
	{        
		int err = ERRNO;
		m_state=STATE_CLOSED;
		return 0;
	}
	if (res > 0)
		return res;
	else
		return 0;
}

/* RingBuffer client function */
size_t JNL_Connection::Write(const void *dest, size_t len)
{
	if (!len)
		return 0;

	int res=socket_send((const char *)dest,len,0);
	if (res==-1 && ERRNO != EWOULDBLOCK)
	{
		return 0;
		//              m_state=STATE_CLOSED;
	}
	if (res > 0)
		return res;
	else
		return 0;
}

void JNL_Connection::reuse()
{
	if (m_state == STATE_CLOSED)
		m_state = STATE_CONNECTED;
}

#define CBCLASS JNL_Connection
START_DISPATCH;
VCB(API_CONNECTION_OPEN, open)
case API_CONNECTION_CONNECT: connect(*(char **)(params[0]), *(int *)(params[1])); return 1;	
VCB(API_CONNECTION_RUN, run)
CB(API_CONNECTION_GETSTATE, get_state)
CB(API_CONNECTION_GETERROR, get_errstr)
VCB(API_CONNECTION_CLOSE, close)
VCB(API_CONNECTION_FLUSHSEND, flush_send)
CB(API_CONNECTION_GETSENDBYTESINQUEUE, send_bytes_in_queue)
CB(API_CONNECTION_GETSENDBYTESAVAILABLE, send_bytes_available)
CB(API_CONNECTION_SEND, send)
CB(API_CONNECTION_SENDBYTES, send_bytes)
CB(API_CONNECTION_SENDSTRING, send_string)
CB(API_CONNECTION_GETRECEIVEBYTESAVAILABLE, recv_bytes_available)
CB(API_CONNECTION_RECEIVEBYTES, recv_bytes)
CB(API_CONNECTION_GETRECEIVELINESAVAILABLE, recv_lines_available)
CB(API_CONNECTION_RECEIVELINE, recv_line)
CB(API_CONNECTION_PEEKBYTES, peek_bytes)
CB(API_CONNECTION_GETINTERFACE, get_interface)
CB(API_CONNECTION_GETREMOTEADDRESS, get_remote)
CB(API_CONNECTION_GETREMOTEPORT, get_remote_port)
END_DISPATCH;
#undef CBCLASS
