#pragma once

#include "jnetlib/jnetlib.h"
#include "nu/PtrList.h"
#include "nu/vector.h"
#include "foundation/dispatch.h"

class ifc_pagegenerator : public Wasabi2::Dispatchable
{
protected:
	ifc_pagegenerator() : Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_pagegenerator() {}
public:	
	int GetData(void *buf, size_t size) { return PageGenerator_GetData(buf, size); }
	int IsNonBlocking() { return PageGenerator_IsNonBlocking(); }
	
private:
	virtual size_t WASABICALL PageGenerator_GetData(void *buf, size_t size)=0;
	virtual int WASABICALL PageGenerator_IsNonBlocking()=0;
	enum
	{
		DISPATCHABLE_VERSION=0,
	};
};

class ifc_connection_callback : public Wasabi2::Dispatchable
{
protected:
	ifc_connection_callback() :  Wasabi2::Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_connection_callback() {}
public:
	ifc_pagegenerator *OnConnection(jnl_http_request_t serv) { return ConnectionCallback_OnConnection(serv); }
	
private:
	virtual ifc_pagegenerator *WASABICALL ConnectionCallback_OnConnection(jnl_http_request_t serv)=0;
		enum
	{
		DISPATCHABLE_VERSION=0,
	};
};

class ConnectionInstance
{
public:
	ConnectionInstance(jnl_connection_t c);
	~ConnectionInstance();

	void close();

	// these will be used by WebServerBaseClass::onConnection yay
	jnl_connection_t connection;
	jnl_http_request_t m_serv;
	ifc_pagegenerator *m_pagegen;
	time_t m_connect_time;
};


class WebServer
{
public:
	WebServer();
	
  virtual ~WebServer();

	void SetConnectionCallback(ifc_connection_callback *_connectionCallback);
  

	// stuff for setting limits/timeouts
  void SetMaxConnections(size_t max_con);
  void SetRequestTimeout(time_t timeout_s);

  // stuff for setting listener port
  int addListenPort(unsigned short port); // TODO: add Protocol Family as parameter
  unsigned short getListenPort(size_t idx, int *err=0);
  void removeListenPort(unsigned short port);
  void removeListenIdx(size_t idx);
	int GetName(size_t index, char *buf, size_t len);

  // call this a lot :)
  void run(void);
	  
  // these can be used externally, as well as are used by the web server
  static void url_encode(char *in, char *out, int max_out);
  static void url_decode(char *in, char *out, int maxlen);
  static void base64decode(char *src, char *dest, int destsize);
  static void base64encode(char *in, char *out);

  static int parseAuth(char *auth_header, char *out, int out_len);//returns 0 on unknown auth, 1 on basic
private:
	enum
	{
		RUN_CONNECTION_CONTINUE=0,
		RUN_CONNECTION_DONE=1,
		RUN_CONNECTION_ERROR=2,
		RUN_CONNECTION_TIMEOUT=3,
	};
  int run_connection(ConnectionInstance *con);
	void add_connection(jnl_connection_t c, jnl_listen_t l);

  time_t m_timeout_s;
  size_t m_max_con;

	nu::PtrList<ConnectionInstance> m_connections;
	nu::HandleList<jnl_listen_t> m_listeners;

  size_t m_listener_rot;
  
	ifc_connection_callback *connectionCallback;
};

