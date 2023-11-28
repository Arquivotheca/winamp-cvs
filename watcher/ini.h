#ifndef NULLSOFT_BASE_INI_HEADER
#define NULLSOFT_BASE_INI_HEADER

#include <windows.h>
#include ".\MLString.h"

class BaseINI
{

public:
	BaseINI(void);
	BaseINI(const wchar_t *file, const wchar_t *section);
	virtual ~BaseINI(void);

public:
    void SetWrokingINI(const wchar_t *file);
	void SetWorkingSection(const wchar_t *section);
	MLString *GetSections(MLString *sections);
	MLString *GetKeys(MLString *keys);
	MLString* GetStringValue(const wchar_t *key, MLString *value, const wchar_t *defVal);
	unsigned int GetIntValue(const wchar_t *key, int defVal);
	void SetValue(const wchar_t *key, const wchar_t *value);
	void SetValue(const wchar_t *key, int &value);
	void SetValue(const wchar_t *key, unsigned int &value);
	HRESULT DeleteSection(void);
	HRESULT DeleteKey(const wchar_t *key);
	void DeleteFile(int onlyEmpty = TRUE);
	void CreateUnicode(int writeSection = TRUE);
protected:
	int IsFileExist(void);
	int HasBOM(HANDLE file);

protected:
	MLString fileName;
	MLString sectionName;

};

#endif // NULLSOFT_ML_INI_HEADER