/*
** JNetLib
** Copyright (C) 2008 Nullsoft, Inc.
** Author: Ben Allison
** File: multicastlisten.cpp - JNL Multicast UDP listen implementation
** License: see jnetlib.h
*/

#include "netinc.h"
#include "util.h"
#include "multicastlisten.h"
#include <strsafe.h>

JNL_MulticastUDPListen::JNL_MulticastUDPListen(const char *mcast_ip, unsigned short port, sockaddr *which_interface, int family)
{
	m_port=port;

	char portString[32];
	StringCbPrintfA(portString, sizeof(portString), "%d", (int)port);

	addrinfo *res=0;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; /* IPv4 only for now until we get IPv6 multicast registration working */
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
	hints.ai_protocol = IPPROTO_UDP;

	if (getaddrinfo(NULL, portString, &hints, &res) == 0)
	{
		m_socket = ::socket(res->ai_family,res->ai_socktype, res->ai_protocol);  
		if (m_socket < 0)
		{
		}
		else
		{
			SET_SOCK_BLOCK(m_socket,0);

			int bflag = 1;
			setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&bflag, sizeof(bflag));
#if defined(__FreeBSD__) || defined(__APPLE__)
			bflag=1; // in case it magically got unset above
			setsockopt(m_socket, SOL_SOCKET, SO_REUSEPORT, (const char *)&bflag, sizeof(bflag));
#endif
			if (::bind(m_socket, res->ai_addr, res->ai_addrlen))
			{
				closesocket(m_socket);
				m_socket=-1;
			}
			else
			{
				// TODO: ipv6 with IPV6_ADD_MEMBERSHIP and ipv6_mreq

				sockaddr_in *ipv4 = (sockaddr_in *)res->ai_addr;

				/* join multicast group */
				ip_mreq ssdpMcastAddr;
				memset(&ssdpMcastAddr, 0, sizeof(ssdpMcastAddr));
				ssdpMcastAddr.imr_interface = ipv4->sin_addr;
				ssdpMcastAddr.imr_multiaddr.s_addr = inet_addr(mcast_ip);
				if (setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&ssdpMcastAddr, sizeof(ssdpMcastAddr)))
				{
					int err = ERRNO;
					return;
				}

				/* Set multicast interface. */
				in_addr addr;
				memset(&addr, 0, sizeof(addr));
				addr = ipv4->sin_addr;
				if (setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (const char *)&addr, sizeof(addr))) 
				{
					int err = ERRNO;
					err=err;
					/* This is probably not a critical error, so let's continue. */
				}

				/* set TTL to 4 */
				uint8_t ttl=4;
				setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl));

				int option = 1;
				if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, (const char *)&option, sizeof(option)) != 0) 
				{
					int err=ERRNO;
					return;
				}
			}
		}
		freeaddrinfo(res);
	}
}

JNL_MulticastUDPListen::~JNL_MulticastUDPListen()
{
	if (m_socket>=0)
	{
		closesocket(m_socket);
	}
}

JNL_UDPConnection *JNL_MulticastUDPListen::get_connect(int sendbufsize, int recvbufsize)
{
	if (m_socket < 0)
	{
		return NULL;
	}

	JNL_UDPConnection *c=new JNL_UDPConnection();
	c->open(m_socket, NULL, sendbufsize, recvbufsize);
	return c;
}
