#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

#define C_CONFIG_WIN32NATIVE

class C_Config
{
  public:
    C_Config(char *ini);
    ~C_Config();
    void Flush(void);
    void  WriteInt(char *name, int value);
    char *WriteString(char *name, char *string);
    int   ReadInt(char *name, int defvalue);
    char *ReadString(char *name, char *defvalue);

  private:
#ifndef C_CONFIG_WIN32NATIVE
    typedef struct 
    {
      char name[16];
      char *value;
    } strType;

    strType *m_strs;
    int m_dirty;
    int m_num_strs, m_num_strs_alloc;
#else
    char m_strbuf[8192];
#endif

    char *m_inifile;
};

#endif//_C_CONFIG_H_
