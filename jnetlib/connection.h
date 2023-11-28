/*
** JNetLib
** Copyright (C) 2000-2007 Nullsoft, Inc.
** Author: Justin Frankel
** File: connection.h - JNL TCP connection interface
** License: see jnetlib.h
**
** Usage:
**   1. Create a JNL_Connection object, optionally specifying a JNL_AsyncDNS
**      object to use (or NULL for none, or JNL_CONNECTION_AUTODNS for auto),
**      and the send and receive buffer sizes.
**   2. Call connect() to have it connect to a host/port (the hostname will be 
**      resolved if possible).
**   3. call run() with the maximum send/recv amounts, and optionally parameters
**      so you can tell how much has been send/received. You want to do this a lot, while:
**   4. check get_state() to check the state of the connection. The states are:
**        JNL_Connection::STATE_ERROR
**          - an error has occured on the connection. the connection has closed,
**            and you can no longer write to the socket (there still might be 
**            data in the receive buffer - use recv_bytes_available()). 
**        JNL_Connection::STATE_NOCONNECTION
**          - no connection has been made yet. call connect() already! :)
**        JNL_Connection::STATE_RESOLVING
**          - the connection is still waiting for a JNL_AsycnDNS to resolve the
**            host. 
**        JNL_Connection::STATE_CONNECTING
**          - the asynchronous call to connect() is still running.
**        JNL_Connection::STATE_CONNECTED
**          - the connection has connected, all is well.
**        JNL_Connection::STATE_CLOSING
**          - the connection is closing. This happens after a call to close,
**            without the quick parameter set. This means that the connection
**            will close once the data in the send buffer is sent (data could
**            still be being received when it would be closed). After it is 
**            closed, the state will transition to:
**        JNL_Connection::STATE_CLOSED
**          - the connection has closed, generally without error. There still
**            might be data in the receieve buffer, use recv_bytes_available().
**   5. Use send() and send_string() to send data. You can use 
**      send_bytes_in_queue() to see how much has yet to go out, or 
**      send_bytes_available() to see how much you can write. If you use send()
**      or send_string() and not enough room is available, both functions will 
**      return error ( < 0)
**   6. Use recv() and recv_line() to get data. If you want to see how much data 
**      there is, use recv_bytes_available() and recv_lines_available(). If you 
**      call recv() and not enough data is available, recv() will return how much
**      data was actually read. See comments at the function defs.
**
**   7. To close, call close(1) for a quick close, or close() for a close that will
**      make the socket close after sending all the data sent. 
**  
**   8. delete ye' ol' object.
*/

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "netinc.h"
#include "api_dns.h"
#include "asyncdns.h"
#include "api_connection.h"
#include "../nu/RingBuffer.h"
#include <stddef.h>

#if defined(_MSC_VER) && (_MSC_VER < 1200)
typedef int intptr_t;
#endif

class JNL_Connection : public api_connection,
	private Filler,
	private Drainer
{
  public:
    typedef enum 
    { 
      STATE_ERROR = CONNECTION_STATE_ERROR,
      STATE_NOCONNECTION = CONNECTION_STATE_NOCONNECTION,
      STATE_RESOLVING = CONNECTION_STATE_RESOLVING,
      STATE_CONNECTING = CONNECTION_STATE_CONNECTING,
      STATE_CONNECTED = CONNECTION_STATE_CONNECTED,
      STATE_CLOSING = CONNECTION_STATE_CLOSING,
      STATE_CLOSED = CONNECTION_STATE_CLOSED,
			STATE_RESOLVED = CONNECTION_STATE_RESOLVED,
    } state;

	/*
	**  Joshua Teitelbaum, 1/27/2006 adding virtual
	*/
	JNL_Connection();
    JNL_Connection(api_dns *dns, size_t sendbufsize, size_t recvbufsize);
    virtual ~JNL_Connection();

public:
	void open(api_dns *dns=API_DNS_AUTODNS, size_t sendbufsize=8192, size_t recvbufsize=8192);
    void connect(char *hostname, int port);
    virtual void connect(int sock, sockaddr *addr, int length /* of addr */); // used by the listen object, usually not needed by users.

	/*
	**  Joshua Teitelbaum 2/2/2006
	**  Need to make this virtual to ensure SSL can init properly
	*/
    virtual void run(int max_send_bytes=-1, int max_recv_bytes=-1, int *bytes_sent=NULL, int *bytes_rcvd=NULL);
    int  get_state() { return m_state; }
    char *get_errstr() { return m_errorstr; }

    void close(int quick=0);
    void flush_send(void) { send_buffer.clear(); }

    size_t send_bytes_in_queue(void);
    size_t send_bytes_available(void);
    int send(const void *data, int length); // returns -1 if not enough room
    inline int send_bytes(const void *data, int length) { return send(data, length); }
    int send_string(const char *line);      // returns -1 if not enough room


    size_t recv_bytes_available(void);
    size_t recv_bytes(void *data, size_t maxlength); // returns actual bytes read
    unsigned int recv_int(void);
    int recv_lines_available(void);
    int recv_line(char *line, int maxlength); // returns 0 if the line was terminated with a \r or \n, 1 if not.
                                              // (i.e. if you specify maxlength=10, and the line is 12 bytes long
                                              // it will return 1. or if there is no \r or \n and that's all the data
                                              // the connection has.)
    size_t peek_bytes(void *data, size_t maxlength); // returns bytes peeked

    unsigned long get_interface(void);        // this returns the interface the connection is on
    unsigned long get_remote(void); // remote host ip.
    short get_remote_port(void); // this returns the remote port of connection
		void set_dns(api_dns *dns);
		void reuse();
  protected:
    intptr_t m_socket;
    short m_remote_port;

		RingBuffer recv_buffer;
		RingBuffer send_buffer;

		addrinfo *saddr;
		sockaddr *address;
    //struct sockaddr_in m_saddr;
    char m_host[256];

		api_dns *m_dns;
    bool m_dns_owned;

    state m_state;
    char *m_errorstr;

   /*
	**  Joshua Teitelbaum 1/27/2006 Adding new BSD socket analogues for SSL compatibility
	*/
	virtual void socket_shutdown();
	virtual ssize_t socket_recv(char *buf, size_t len, int options);
    virtual int socket_send(const char *buf, int len, int options);
	virtual int socket_connect();
	virtual void on_socket_connected();
private:
	void init(); // constructor helper function
	
		// functions for RingBuffer
		size_t Read(void *dest, size_t len);
		size_t Write(const void *dest, size_t len);
	protected:
		RECVS_DISPATCH;
};
#endif // _Connection_H_
