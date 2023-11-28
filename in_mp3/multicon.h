#ifndef __MULTICON_H
#define __MULTICON_H

#include "../jnetlib/api_dns.h"
#include "../jnetlib/api_connection.h"

#define MAX_CONS 8

class MTCP_Connect {
public:	
	MTCP_Connect( char *url, api_dns *dns, int sendbufsize, int recvbufsize, char *http_ver_str = " HTTP/1.0\r\n" , int start_offset = 0, char *proxyconfig=NULL);
	~MTCP_Connect();

	void send_string( char *str );
	int tryConnect( api_connection **con, char *host, int *port, char *request );
	static int isMTCP( char *url ) { return strstr( url, ";uvox://" ) || strstr( url, "<>" ); }

private:
	int parseUrl( char *url, char *host, int *port, char *request );

	struct cons_data {
		api_connection *con;
		char host[256];
		char request[2048];
		int port;
	} m_cons[MAX_CONS];
	int m_ncons;
	int connected;
	int serialized;
};

#endif