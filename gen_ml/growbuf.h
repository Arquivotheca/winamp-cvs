#ifndef _GROWBUF_H_
#define _GROWBUF_H_


class GrowBuf
{
public:
  GrowBuf() {
    m_buf=NULL;
    m_alloc=0;
    m_size=0;
  }
  ~GrowBuf() {
    free(m_buf);
  }
  char *Get() {
    return m_buf;
  }

  void Append(const char *append, int size) {
    Grow(m_size+size);
    memcpy(m_buf+m_size,append,size);
    m_size+=size;
  }
  void Grow(int newsize) {
    if (m_alloc < newsize) {
      m_alloc=newsize*2;
      m_buf=(char*)realloc(m_buf,m_alloc);
    }
  }
  void Clear() {
    free(m_buf);
    m_buf=NULL;
    m_alloc=0;
    m_size=0;
  }
private:

  char *m_buf;
  int m_alloc;
  int m_size;
};


#endif//_GAYSTRING_H_