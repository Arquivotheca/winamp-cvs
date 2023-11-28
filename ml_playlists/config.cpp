#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>


C_Config::~C_Config()
{
  free(m_inifile);
}

C_Config::C_Config(char *ini)
{
  m_inifile=_strdup(ini);
}

void C_Config::WriteInt(char *name, int value)
{
  char buf[32];
  sprintf(buf,"%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(char *name, int defvalue)
{
  return GetPrivateProfileIntA("gen_ml_config",name,defvalue,m_inifile);
}

char *C_Config::WriteString(char *name, char *string)
{
  WritePrivateProfileStringA("gen_ml_config",name,string,m_inifile);
  return name;
}

char *C_Config::ReadString(char *name, char *defstr)
{
  static char foobuf[] = "___________gen_ml_lameness___________";
  m_strbuf[0]=0;
  GetPrivateProfileStringA("gen_ml_config",name,foobuf,m_strbuf,sizeof(m_strbuf),m_inifile);
  if (!strcmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[sizeof(m_strbuf)-1]=0;
  return m_strbuf;
}
