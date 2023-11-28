#ifndef NULLSOFT_UTILITY_CONFIGH
#define NULLSOFT_UTILITY_CONFIGH
#include <string>
#include <map>
#include <windows.h>
#if defined(UNICODE) || defined(_UNICODE)
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif
namespace Nullsoft
{
	namespace Utility
	{
		template <class config_t>
		class ConfigItem
		{
		public:
			ConfigItem(tstring &_appName, tstring &_fileName, LPCTSTR _keyName)
					: appName(&_appName), fileName(&_fileName), keyName(_keyName)
			{}

			void operator =(config_t input)
			{
				WritePrivateProfileStruct(appName->c_str(),
				                          keyName.c_str(),
				                          (void *) & input,
				                          sizeof(input),
				                          fileName->c_str());
			}

			operator config_t()
			{
				config_t temp;

				memset(&temp, 0, sizeof(temp));
				GetPrivateProfileStruct(appName->c_str(),
				                        keyName.c_str(),
				                        &temp,
				                        sizeof(temp),
				                        fileName->c_str());
				return temp;
			}

			tstring *appName, *fileName;
			tstring keyName;

		};


		template <>
		class ConfigItem<TCHAR *>
		{
		public:
			ConfigItem(tstring &_appName, tstring &_fileName, LPCTSTR _keyName)
					: appName(&_appName), fileName(&_fileName), keyName(_keyName)
			{}
			void operator =(LPCTSTR input)
			{
				WritePrivateProfileString(appName->c_str(),
				                          keyName.c_str(),
				                          input,
				                          fileName->c_str());
			}
			void GetString(LPTSTR str, size_t len)
			{
				GetPrivateProfileString(appName->c_str(), keyName.c_str(), TEXT(""), str, len,fileName->c_str());
			}
			tstring *appName, *fileName;
			tstring keyName;
		};

		template <>
		class ConfigItem<int>
		{
		public:
			ConfigItem(tstring &_appName, tstring &_fileName, LPCTSTR _keyName)
					: appName(&_appName), fileName(&_fileName), keyName(_keyName), def(0)
			{}

			void operator =(int input)
			{
				TCHAR tmp[(sizeof(int) / 2) * 5 + 1]; // enough room to hold for 16,32 or 64 bit ints, plus null terminator
				wsprintf(tmp, TEXT("%d"), input);
				WritePrivateProfileString(appName->c_str(),
				                          keyName.c_str(),
				                          tmp,
				                          fileName->c_str());
			}

			operator int ()
			{
				return GetPrivateProfileInt(appName->c_str(), keyName.c_str(), def, fileName->c_str());
			}

			void SetDefault(int _def)
			{
				def = _def;
			}
			tstring *appName, *fileName;
			tstring keyName;
			int def;
		};

		class Config
		{
		public:
			Config()
			{
			}

			void SetFile(LPCTSTR iniFile, LPCTSTR _appName)
			{
				appName = _appName;
				fileName =iniFile;
			}

			ConfigItem<int> cfg_int(LPCTSTR keyName, int def)
			{
				ConfigItem<int> item(appName, fileName, keyName);
				item.SetDefault(def);
				return item;
			}

			ConfigItem<TCHAR *> cfg_str(LPCTSTR keyName)
			{
				return ConfigItem<TCHAR *>(appName, fileName, keyName);
			}

			ConfigItem<GUID> cfg_guid(LPCTSTR keyName)
			{
				return ConfigItem<GUID>(appName, fileName, keyName);

			}
			ConfigItem<__int64> cfg_int64(LPCTSTR keyName)
			{
				ConfigItem<__int64> item(appName, fileName, keyName);
				return item;
			}

			tstring appName, fileName;
		};
	}
}
#endif
