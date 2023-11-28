#pragma once
#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "nx/nxstring.h"
// ----------------------------------------------------------------------------

class ifc_xmlattributes : public Wasabi2::Dispatchable
{
  protected:
    ifc_xmlattributes() : Dispatchable(DISPATCHABLE_VERSION) {}
    ~ifc_xmlattributes() {}
  public:
		const nsxml_char_t *GetAttribute(nx_string_t name) { return XMLAttributes_GetAttribute(name); }
    //const wchar_t *getItemName(size_t i);
    //const wchar_t *getItemValue(size_t i);
    //const wchar_t *getItemValue(const wchar_t *name);
    //const wchar_t *enumItemValues(const wchar_t *name, size_t nb);
		//int getItemValueInt(const wchar_t *name, int def = 0);
    //size_t getNbItems();
  
  enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual const nsxml_char_t *WASABICALL XMLAttributes_GetAttribute(nx_string_t name)=0;
};
