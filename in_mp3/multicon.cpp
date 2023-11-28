#ifdef MTCP_CONNECT
#include <windows.h>
#include "stdio.h"
#include "multicon.h"
#include "main.h"

#include "api.h"
#include <api/service/waservicefactory.h>

static waServiceFactory *connectionFactory = 0;
static api_connection *CreateConnection()
{
	api_connection *conn = 0;
	if (!connectionFactory && WASABI_API_SVC)
		connectionFactory = WASABI_API_SVC->service_getServiceByGuid(connectionFactoryGUID);

	if (connectionFactory)
		conn = (api_connection *)connectionFactory->getInterface();

	return conn;
}

static void ReleaseConnection(api_connection *&conn)
{
	if (!conn)
		return ;

	if (!connectionFactory && WASABI_API_SVC)
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(connectionFactoryGUID);

	if (connectionFactory)
		connectionFactory->releaseInterface(conn);

	conn = 0;
}


static void parseURL(char *url, char *lp, char *host, int *port, char *req)
{
  char *p,*np;
/*  if (_strnicmp(url,"http://",4) &&
	    _strnicmp(url,"icy://",6) &&
	    _strnicmp(url,"sc://",6) &&
	    _strnicmp(url,"shoutcast://",12)) return;
      */
  np=p=strstr(url,"://");
  if (!np) np=(char*)url;
  else np+=3;
  if (!p) p=(char*)url;
  else p+=3;

  while (*np != '/' && *np) *np++;
  if (*np)
  {
    lstrcpyn(req,np,2048);
    *np++=0;
  } 
  else strcpy(req,"/");
  np=p;
  while (*np != '@' && *np) np++;
  if (*np)
  {
    *np++=0;
    lstrcpyn(lp,p,256);
    p=np;
  }
  else lp[0]=0;
  np=p;
  while (*np != ':' && *np) np++;
  if (*np)
  {
    *np++=0;
    *port=atoi(np);
  } else *port=80;
  lstrcpyn(host,p,256);
}

static void encodeMimeStr(char *in, char *out)
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
    *out++ = alphabet[(accum & 0xF)<<2];
    *out++='=';  
  }
  else if (shift == 2)
  {
    *out++ = alphabet[(accum & 0x3)<<4];
    *out++='=';  
    *out++='=';  
  }

  *out++=0;
}

MTCP_Connect::MTCP_Connect( char *url, api_dns *dns, int sendbufsize, int recvbufsize, char *http_ver_str, int start_offset, char *proxyconfig  ) 
{
	char tmp[8192];
	char woot[8192];
  __int64 mythen, mynow , myref;
  LARGE_INTEGER then,now, ref;
  int timeout = 0,timer=0;
	lstrcpyn(tmp, url, 8192);
	char *cur = tmp;
  char *temp = strstr( cur, "order://");
    char req[512];
    char proxy_lp[512]="";
    char proxy_host[512]="";
    int proxy_port=80;

	serialized = 0;

  if ( temp )
  {
    char *p = temp;
    p+=8;
    *temp = '\0';
    timeout = atoi(p);
    if ( timeout ) serialized = 1;
    
    QueryPerformanceFrequency( &ref);
    myref = ref.QuadPart;


  }
  connected = 0;

	for( m_ncons = 0; cur && m_ncons < MAX_CONS; m_ncons++ ) 
  {
		char *next = strstr( cur, "<>" );
		if ( next ) {
			*next = '\0';
			next += 2;

		} else {
			next = strstr( cur, ";uvox://" );
			if ( next ) {
				*next = '\0';
				next++;
			}
		}

		if ( proxyconfig )
		{
			parseURL(proxyconfig,proxy_lp,proxy_host,&proxy_port,req);
		}

		if ( parseUrl( cur, m_cons[m_ncons].host, &m_cons[m_ncons].port, m_cons[m_ncons].request ) ) 
    {
			m_cons[m_ncons].con = CreateConnection();
			if (!m_cons[m_ncons].con)
				break;
			m_cons[m_ncons].con->Open(dns, sendbufsize, recvbufsize);
			//m_cons[m_ncons].con->connect( m_cons[m_ncons].host, m_cons[m_ncons].port );

	    if (!proxy_host[0])
		  {
			   m_cons[m_ncons].con->Connect(m_cons[m_ncons].host,m_cons[m_ncons].port);
	       m_cons[m_ncons].con->SendString("GET ");
				 m_cons[m_ncons].con->SendString(m_cons[m_ncons].request);
				 m_cons[m_ncons].con->SendString(http_ver_str);
			}
			else
			{
	      char s[32];
				m_cons[m_ncons].con->Connect(proxy_host,proxy_port);
				m_cons[m_ncons].con->SendString("GET http://");
				m_cons[m_ncons].con->SendString(m_cons[m_ncons].host);
				wsprintf(s,":%d",m_cons[m_ncons].port);
				m_cons[m_ncons].con->SendString(s);
				m_cons[m_ncons].con->SendString(m_cons[m_ncons].request);
				m_cons[m_ncons].con->SendString(http_ver_str);
				if (proxy_lp[0])
				{
	        char temp[1024];
					m_cons[m_ncons].con->SendString("Proxy-Authorization: Basic ");
					encodeMimeStr(proxy_lp,temp);
					m_cons[m_ncons].con->SendString(temp);
					m_cons[m_ncons].con->SendString("\r\n");
	
				}
			}

			m_cons[m_ncons].con->SendString("Host: " );
			m_cons[m_ncons].con->SendString(m_cons[m_ncons].host);
			m_cons[m_ncons].con->SendString("\r\n");

				
      if ( serialized )
      {
        timer = 0;         

        m_cons[m_ncons].con->SendString(GetUltravoxUserAgent());

			  m_cons[m_ncons].con->SendString("Accept: */*\r\n");
      	if (start_offset>0)
			  {				
					char str[512];
				  wsprintf(str,"Range: bytes=%d-\r\n",start_offset);
				  m_cons[m_ncons].con->SendString(str);
			  }
			  m_cons[m_ncons].con->SendString("Connection: close\r\n");
			  m_cons[m_ncons].con->SendString("\r\n");


        QueryPerformanceCounter( &then );
        mythen = then.QuadPart;
        while ( timer < timeout && m_cons[m_ncons].con )
        {
          m_cons[m_ncons].con->Run();

		      if (m_cons[m_ncons].con->GetState()==CONNECTION_STATE_CLOSED ||
                m_cons[m_ncons].con->GetState()==CONNECTION_STATE_ERROR ||
                m_cons[m_ncons].con->GetState()==CONNECTION_STATE_NOCONNECTION ) 
          {
						ReleaseConnection(m_cons[m_ncons].con);
							break;
		      } 
          else if ( m_cons[m_ncons].con->GetReceiveLinesAvailable() > 1 ) 
          {
			      int len = m_cons[m_ncons].con->PeekBytes( woot, sizeof( woot ) );
			      woot[len] = '\0';
			      if ( strstr( woot, "200 OK" ) || strstr( woot, "206 Partial Content" ) ) 
            {

    			    connected = 1;
              break;
		  	    }
						else 
						{
							ReleaseConnection(m_cons[m_ncons].con);
						  break;
						}

          }
      
          QueryPerformanceCounter( &now );
          mynow = now.QuadPart;

          float profiletime = mynow - mythen;
			    profiletime /= myref;
		      profiletime *= 1000.0;
          timer = (int) profiletime;
          Sleep(1);

        } // while

        if ( connected ) break;
        if ( timer >= timeout ) 
				{
						ReleaseConnection(m_cons[m_ncons].con);
				}

      }

		}

		cur = next;
    if ( connected ) break;
	}

}

MTCP_Connect::~MTCP_Connect() {
	if ( ! connected ) {
		for( int i = 0; i < m_ncons; i++ )
			ReleaseConnection(m_cons[i].con);
	}
}

void MTCP_Connect::send_string( char *str ) {
	for( int i = 0; i < m_ncons; i++ )
		if ( m_cons[i].con )
			m_cons[i].con->SendString(str);
}

int MTCP_Connect::tryConnect( api_connection **con, char *host, int *port, char *request ) 
{
	char tmp[4192];

  if ( connected )
  {
    *con = m_cons[m_ncons].con;
		strcpy( host, m_cons[m_ncons].host );
		strcpy( request, m_cons[m_ncons].request );
		*port = m_cons[m_ncons].port;
    return 1;
  }
	if ( serialized && !connected )
	{
		return -1;
	}

	for( int i = 0; i < m_ncons; i++ ) {
		m_cons[i].con->Run();

		if (m_cons[i].con->GetState()==CONNECTION_STATE_CLOSED ||
              m_cons[i].con->GetState()==CONNECTION_STATE_ERROR ||
              m_cons[i].con->GetState()==CONNECTION_STATE_NOCONNECTION ) {
			ReleaseConnection(m_cons[i].con);
			m_cons[i] = m_cons[--m_ncons];
			i--;
		} else if ( m_cons[i].con->GetReceiveLinesAvailable() > 0 ) {
			int len = m_cons[i].con->PeekBytes( tmp, sizeof( tmp ) );
			tmp[len] = '\0';

			if ( strstr( tmp, "200 OK" ) || strstr( tmp, "206 Partial Content" ) ) {
				*con = m_cons[i].con;
				strcpy( host, m_cons[i].host );
				strcpy( request, m_cons[i].request );
				*port = m_cons[i].port;

				connected = 1;

				for( int j = 0; j < m_ncons; j++ ) {
					if ( j != i && m_cons[j].con )
						ReleaseConnection(m_cons[j].con);
				}

				return 1;
			}
		}
	}

	return m_ncons>0?0:-1;
}

int MTCP_Connect::parseUrl(char *url, char *host, int *port, char *req) 
{
	char lp[256];
  char *p,*np;
/*  if (_strnicmp(url,"http://",4) &&
	    _strnicmp(url,"icy://",6) &&
	    _strnicmp(url,"sc://",6) &&
	    _strnicmp(url,"shoutcast://",12)) return;
      */
  np=p=strstr(url,"://");
  if (!np) np=(char*)url;
  else np+=3;
  if (!p) p=(char*)url;
  else p+=3;

  while (*np != '/' && *np) *np++;
  if (*np)
  {
    lstrcpyn(req,np,2048);
    *np++=0;
  } 
  else strcpy(req,"/");
  np=p;
  while (*np != '@' && *np) np++;
  if (*np)
  {
    *np++=0;
    lstrcpyn(lp,p,256);
    p=np;
  }
  else lp[0]=0;
  np=p;
  while (*np != ':' && *np) np++;
  if (*np)
  {
    *np++=0;
    *port=atoi(np);
  } else *port=80;
  lstrcpyn(host,p,256);
  
	return !!host[0];
}

#endif