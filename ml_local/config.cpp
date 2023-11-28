#include "main.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "config.h"

void C_Config::Flush(void)
{
}

C_Config::~C_Config()
{
  free(m_inifile);
}

C_Config::C_Config(char *ini)
{
  m_strbuf[0]=0;
  m_inifile=_strdup(ini);
}

void C_Config::WriteInt(const char *name, int value)
{
  char buf[32];
  sprintf(buf,"%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(const char *name, int defvalue)
{
  return GetPrivateProfileIntA("gen_ml_config",name,defvalue,m_inifile);
}

const char *C_Config::WriteString(const char *name, const char *string)
{
  WritePrivateProfileStringA("gen_ml_config",name,string,m_inifile);
  return name;
}

char *C_Config::ReadString(const char *name, char *defstr)
{
  static char foobuf[] = "mllm_empty";
  m_strbuf[0]=0;
  GetPrivateProfileStringA("gen_ml_config",name,foobuf,m_strbuf,sizeof(m_strbuf),m_inifile);
  if (!strcmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[sizeof(m_strbuf)-1]=0;
  return m_strbuf;
}

bool C_Config::ReadString(const char *name, const char *defvalue, char *storage, size_t len)
{
	static char foobuf[] = "mllm_empty";
	storage[0]=0;
	GetPrivateProfileStringA("gen_ml_config",name,foobuf,storage,len,m_inifile);
	if (!strcmp(foobuf,storage))
	{
		if (defvalue)
			lstrcpyn(storage, defvalue, len);
		return false;
	}
	return true;
}