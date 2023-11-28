/*
** JNetLib
** Copyright (C) 2000-2001 Nullsoft, Inc.
** Author: Justin Frankel
** File: listen.cpp - JNL TCP listen implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "listen.h"

JNL_Listen::JNL_Listen(unsigned short port, sockaddr *which_interface, int family)
{
	m_port=port;

	char portString[32];
	sprintf(portString, "%d", (int)port);

	addrinfo *res;

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
	//hints.ai_addr = which_interface;

	if (getaddrinfo(NULL, portString, &hints, &res) == 0)
	{
		m_socket = ::socket(res->ai_family,res->ai_socktype, res->ai_protocol);  
	  if (m_socket < 0) 
	  {
	  }
	  else
	  {
    SET_SOCK_BLOCK(m_socket,0);
#ifndef _WIN32
    int bflag = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &bflag, sizeof(bflag));
#endif
		if (::bind(m_socket, which_interface?which_interface:res->ai_addr, res->ai_addrlen))
    {
      closesocket(m_socket);
      m_socket=-1;
    }
    else if (::listen(m_socket,8)==-1) 
      {
        closesocket(m_socket);
        m_socket=-1;
      }
  }
		freeaddrinfo(res);
	}
}

JNL_Listen::~JNL_Listen()
{
  if (m_socket>=0)
  {
    closesocket(m_socket);
  }
}

JNL_Connection *JNL_Listen::get_connect(int sendbufsize, int recvbufsize)
{
  if (m_socket < 0)
  {
    return NULL;
  }
	sockaddr_storage saddr;
	socklen_t length = sizeof(saddr);
	intptr_t s = accept(m_socket, (sockaddr *)&saddr, &length);
  if (s != -1)
  {
    JNL_Connection *c=new JNL_Connection(NULL,sendbufsize, recvbufsize);
    c->connect(s, (sockaddr *)&saddr, length);
    return c;
  }
	
  return NULL;
}


socklen_t JNL_Listen::get_address(sockaddr* address, socklen_t *address_len)
{
	return getsockname(m_socket, address, address_len);
}