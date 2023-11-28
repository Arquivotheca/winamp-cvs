#ifndef _C_CONFIG_H_
#define _C_CONFIG_H_

#define C_CONFIG_WIN32NATIVE

class C_Config
{
  public:
    C_Config(char *ini);
    ~C_Config();
    void Flush(void);
    void  WriteInt(const char *name, int value);
    const char *WriteString(const char *name, const char *string);
    int   ReadInt(const char *name, int defvalue);
    char *ReadString(const char *name, char *defvalue);
		bool ReadString(const char *name, const char *defvalue, char *storage, size_t len); // returns true if value was found
  private:
    char m_strbuf[8192];
    char *m_inifile;
};

#endif//_C_CONFIG_H_
