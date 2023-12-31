/*
** JNetLib
** Copyright (C) 2000-2006 CockOS, Inc.
** Author: Justin Frankel, Joshua Teitelbaum
** File: sslconnection.h - JNL SSL TCP connection interface
** License: see jnetlib.h
*/

#ifndef _JNETLIB_SSL_H_
#define _JNETLIB_SSL_H_
/*
**  AUTOLINK WITH THESE GUYS NOT IN PROJECT HEH HEH :)
**  Build and go, for you :)
*/

#pragma comment(lib,"libeay32.lib")
#pragma comment(lib,"ssleay32.lib")
#pragma comment(lib,"ADVAPI32.LIB")
#pragma comment(lib,"USER32.LIB")

#include <openssl/ssl.h>
#include "connection.h"


class JNL_SSL_Connection : public JNL_Connection
{
protected:
	SSL *m_ssl;
	bool m_bcontextowned;
	bool m_bsslinit;
public:
	JNL_SSL_Connection();
	JNL_SSL_Connection(SSL* pssl ,api_dns *dns, size_t sendbufsize, size_t recvbufsize);
	virtual ~JNL_SSL_Connection();
	virtual void connect(int sock, sockaddr *addr, int length /* of addr */); // used by the listen object, usually not needed by users.
  virtual void run(int max_send_bytes=-1, int max_recv_bytes=-1, int *bytes_sent=NULL, int *bytes_rcvd=NULL);

	/*
	**  Joshua Teitelbaum 1/27/2006 Adding new BSD socket analogues for SSL compatibility
	*/
protected:
	virtual void socket_shutdown();
	virtual ssize_t socket_recv(char *buf, size_t len, int options);
    virtual int socket_send(const char *buf, int len, int options);
	virtual int socket_connect();
	virtual void on_socket_connected();
	/*
	**  init_ssl_connection:
	**  returns true if can continue onwards (could be error, in which case
	**  we want the error to cascade through).
	**  Else, false if connection was not made, and we have to call this
	**  again.
	*/
	bool init_ssl_connection();

	bool forceConnect;
};

 extern SSL_CTX *sslContext;
#endif
