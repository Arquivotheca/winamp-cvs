#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "config.h"

C_Config::~C_Config()
{
  free(m_inifile);
}

C_Config::C_Config(wchar_t *ini)
{
  m_inifile=_wcsdup(ini);
}

void C_Config::WriteInt(wchar_t *name, int value)
{
  wchar_t buf[32];
  _swprintf(buf,L"%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(wchar_t *name, int defvalue)
{
  return GetPrivateProfileIntW(L"gen_ml_config",name,defvalue,m_inifile);
}

wchar_t *C_Config::WriteString(wchar_t *name, wchar_t *string)
{
  WritePrivateProfileStringW(L"gen_ml_config",name,string,m_inifile);
  return name;
}

wchar_t *C_Config::ReadString(wchar_t *name, wchar_t *defstr)
{
  static wchar_t foobuf[] = L"___________gen_ml_lameness___________";
  m_strbuf[0]=0;
  GetPrivateProfileStringW(L"gen_ml_config",name,foobuf,m_strbuf,sizeof(m_strbuf),m_inifile);
  if (!wcscmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[sizeof(m_strbuf)-1]=0;
  return m_strbuf;
}
