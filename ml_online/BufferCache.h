#ifndef NULLSOFT_BUFFERCACHEH
#define NULLSOFT_BUFFERCACHEH
#include <time.h>

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};


class Buffer_GrowBuf
{
  public:
    Buffer_GrowBuf() { m_alloc=m_used=0; m_s=NULL; }
    ~Buffer_GrowBuf() 
    {
      if (m_s) free(m_s);
      m_s=0; 
    }

    size_t add(void *data, size_t len) 
    { 
      if (len<=0) return 0;
      resize(m_used+len); 
      memcpy((char*)get()+m_used-len,data,len);
      return m_used-len;
    }

    void set(void *data, size_t len)
    {
      resize(len);
      memcpy((char*)get(),data,len);
    }

    void resize(size_t newlen)
    {
      m_used=newlen;
      if (newlen > m_alloc)
      {
        void *ne;
        m_alloc = newlen*2;
        if (m_alloc < 1024) m_alloc =1024;
        ne = realloc(m_s, m_alloc);
        if (!ne)
        {
          ne=malloc(m_alloc);
          if (!ne) *((char*)ne)=NULL;
          memcpy(ne,m_s,m_used);
          free(m_s);
        }
        m_s=ne;
      }
    }

    size_t getlen() { return m_used; }
    void *get() { return m_s; }

    time_t expire_time;

private:
    void *m_s;
    size_t m_alloc;
    size_t m_used;
   
    

};

#endif