/*
** JNetLib
** Copyright (C) 2000-2001 Nullsoft, Inc.
** Author: Justin Frankel
** File: udpconnection.h - JNL UDP connection interface
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

#ifndef _UDPCONNECTION_H_
#define _UDPCONNECTION_H_

#include "asyncdns.h"
#include "api_connection.h"
#include "../nu/RingBuffer.h"

class JNL_UDPConnection :
	private Drainer,
	private Filler
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

		JNL_UDPConnection();
    JNL_UDPConnection(unsigned short port, JNL_AsyncDNS *dns, int sendbufsize=8192, int recvbufsize=8192);
		void open(api_dns *dns=API_DNS_AUTODNS, size_t sendbufsize=8192, size_t recvbufsize=8192);
		void open(int incoming_socket, api_dns *dns=API_DNS_AUTODNS, size_t sendbufsize=8192, size_t recvbufsize=8192);
    ~JNL_UDPConnection();

    void setpeer(char *hostname, int port);
    void setpeer(sockaddr *addr, int length /* of addr */);

    void run(int max_send_bytes=-1, int max_recv_bytes=-1, int *bytes_sent=NULL, int *bytes_rcvd=NULL);
    int  get_state() { return m_state; }
    char *get_errstr() { return m_errorstr; }

    void close(int quick=0);
    void flush_send(void) { send_buffer.clear(); }

    int send_bytes_in_queue(void);
    int send_bytes_available(void);
    int send(char *data, int length); // returns -1 if not enough room
    int send_string(char *line);      // returns -1 if not enough room


    int recv_bytes_available(void);
    int recv_bytes(char *data, int maxlength); // returns actual bytes read
    unsigned int recv_int(void);
    int recv_lines_available(void);
    int recv_line(char *line, int maxlength); // returns 0 if the line was terminated with a \r or \n, 1 if not.
                                              // (i.e. if you specify maxlength=10, and the line is 12 bytes long
                                              // it will return 1. or if there is no \r or \n and that's all the data
                                              // the connection has.)
    int peek_bytes(char *data, int maxlength); // returns bytes peeked

    int get_interface(sockaddr *sin, socklen_t *sin_length); // this returns the interface the connection is on
    short get_remote_port(void) { return m_remote_port; } // this returns the remote port of connection

    void get_last_recv_msg_addr(sockaddr **addr, socklen_t *len) { *addr=(sockaddr *)&m_last_addr; *len=m_last_addr_len; }
		void set_ttl(uint8_t new_ttl);
  protected:
		uint8_t ttl;
    int m_socket;
    short m_remote_port;
		RingBuffer recv_buffer;
		RingBuffer send_buffer;
		/*
    char *m_recv_buffer;
    char *m_send_buffer;
    int m_recv_buffer_len;
    int m_send_buffer_len;

    int  m_recv_pos;
    int  m_recv_len;
    int  m_send_pos;
    int  m_send_len;
*/
		sockaddr *address;
		socklen_t address_len;
    //sockaddr m_saddr;
    sockaddr_storage m_last_addr;
		socklen_t m_last_addr_len;
    addrinfo *saddr;
		
    char m_host[256];

    api_dns *m_dns;
    int m_dns_owned;

    state m_state;
    char *m_errorstr;

private:
		void init(); // constructor helper function

		// functions for RingBuffer
		size_t Read(void *dest, size_t len);
		size_t Write(const void *dest, size_t len);

};

#endif // _UDPConnection_H_
