#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

class C_Config
{
  public:
    C_Config(wchar_t *ini);
    ~C_Config();
    void WriteInt(wchar_t *name, int value);
    wchar_t *WriteString(wchar_t *name, wchar_t *string);
    int ReadInt(wchar_t *name, int defvalue);
    wchar_t *ReadString(wchar_t *name, wchar_t *defvalue);

  private:
    wchar_t m_strbuf[8192];
    wchar_t *m_inifile;
};

#endif//_C_CONFIG_H_