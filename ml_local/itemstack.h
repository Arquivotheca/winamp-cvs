
#ifndef _C_ITEMSTACK_H_
#define _C_ITEMSTACK_H_

template<class T> class C_ItemStack 
{
  public:
    C_ItemStack()
    {
      m_size=0;
      m_list=NULL;
    }
    ~C_ItemStack()
    {
      if (m_list) ::free(m_list);
    }

    void Push(T &i)
    {
      if (!m_list || !(m_size&31))
      {
        m_list=(T*)::realloc(m_list,sizeof(T)*(m_size+32));
      }
      m_list[m_size++]=i;
    }

    int Pop(T &r) 
    { 
      if (!m_size || !m_list) return 1;
      r=m_list[--m_size];
      return 0;
    }

    void Clear()
    {
      m_size=0;
      free(m_list);
      m_list=0;
    }

    int GetSize() { return m_size; }

  protected:
    T *m_list;
    int m_size;

};

#endif //_C_ITEMSTACK_H_
