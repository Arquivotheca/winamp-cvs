#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "config.h"

void C_Config::Flush(void)
{
#ifndef C_CONFIG_WIN32NATIVE
  if (!m_dirty) return;
  FILE *fp=fopen(m_inifile,"wt");
  if (!fp) return;

  fprintf(fp,"[gen_ml_config]\n");
  int x;
  if (m_strs) 
  {
    for (x = 0; x < m_num_strs; x ++)
    {
      char name[17];
      memcpy(name,m_strs[x].name,16);
      name[16]=0;
      if (m_strs[x].value) fprintf(fp,"%s=%s\n",name,m_strs[x].value);
    }
  }
  fclose(fp);
  m_dirty=0;
#endif
}

C_Config::~C_Config()
{
#ifndef C_CONFIG_WIN32NATIVE
  int x;
  Flush();
  if (m_strs) for (x = 0; x < m_num_strs; x ++) free(m_strs[x].value);
  free(m_strs);
#endif

  free(m_inifile);
}

C_Config::C_Config(char *ini)
{
  m_inifile=_strdup(ini);

#ifndef C_CONFIG_WIN32NATIVE
  m_dirty=0;
  m_strs=NULL;
  m_num_strs=m_num_strs_alloc=0;

  // read config
  FILE *fp=fopen(m_inifile,"rt");
  if (!fp) return;

  for (;;)
  {
    char buf[4096];
    buf[0]=0;
    fgets(buf,sizeof(buf),fp);
    if (!buf[0] || feof(fp)) break;
    for (;;)
    {
      int l=strlen(buf);
      if (l > 0 && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[l-1]=0;
      else break;
    }
    if (buf[0] != '[') 
    {
      char *p=strstr(buf,"=");
      if (p)
      {
        *p++=0;
        WriteString(buf,p);
      }
    }
  }
  m_dirty=0;
  fclose(fp);
#endif
}

void C_Config::WriteInt(char *name, int value)
{
  char buf[32];
  sprintf(buf,"%d",value);
  WriteString(name,buf);
}

int C_Config::ReadInt(char *name, int defvalue)
{
#ifndef C_CONFIG_WIN32NATIVE
  char *t=ReadString(name,"");
  if (*t) return atoi(t);
  return defvalue;
#else
  return GetPrivateProfileInt("gen_ml_config",name,defvalue,m_inifile);
#endif
}

char *C_Config::WriteString(char *name, char *string)
{
#ifndef C_CONFIG_WIN32NATIVE
  int x;
  m_dirty=1;
  for (x = 0; x < m_num_strs; x ++)
  {
    if (m_strs[x].value && !strncmp(name,m_strs[x].name,16))
    {
      unsigned int l=(strlen(m_strs[x].value)+16)&~15;
      if (strlen(string)<l)
      {
        strcpy(m_strs[x].value,string);
      }
      else
      {
        free(m_strs[x].value);
        m_strs[x].value = (char *)malloc((strlen(string)+16)&~15);
        strcpy(m_strs[x].value,string);
      }
      return m_strs[x].value;
    }
  }

  // not already in there
  if (m_num_strs >= m_num_strs_alloc || !m_strs)
  {
    m_num_strs_alloc=m_num_strs*3/2+8;
    m_strs=(strType*)::realloc(m_strs,sizeof(strType)*m_num_strs_alloc);
  }
  strncpy(m_strs[m_num_strs].name,name,16);
  m_strs[m_num_strs].value = (char *)malloc((strlen(string)+16)&~15);
  strcpy(m_strs[m_num_strs].value,string);
  return m_strs[m_num_strs++].value;
#else
  WritePrivateProfileString("gen_ml_config",name,string,m_inifile);
  return name;
#endif
}

char *C_Config::ReadString(char *name, char *defstr)
{
#ifndef C_CONFIG_WIN32NATIVE
  int x;
  for (x = 0; x < m_num_strs; x ++)
  {
    if (m_strs[x].value && !::strncmp(name,m_strs[x].name,16))
    {
      return m_strs[x].value;
    }
  }
  return defstr;
#else
  static char foobuf[] = "_gen_ml_empty_";
  m_strbuf[0]=0;
  GetPrivateProfileString("gen_ml_config",name,foobuf,m_strbuf,sizeof(m_strbuf),m_inifile);
  if (!strcmp(foobuf,m_strbuf)) return defstr;

  m_strbuf[sizeof(m_strbuf)-1]=0;
  return m_strbuf;
#endif
}
