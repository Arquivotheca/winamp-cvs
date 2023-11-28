#pragma once

#include "foundation/dispatch.h"
#include "foundation/types.h"
#include "ifc_xmlattributes.h"

class ifc_xmlcallback : public Wasabi2::Dispatchable
{
protected:
	ifc_xmlcallback() : Dispatchable(DISPATCHABLE_VERSION) {}
	~ifc_xmlcallback() {}

public:
	void OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes) { return XMLCallback_OnStartElement(xmlpath, xmltag, attributes); }
	void OnEndElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag) { return XMLCallback_OnEndElement(xmlpath, xmltag); }
	void OnCharacterData(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, const nsxml_char_t *characters, size_t num_characters) { return XMLCallback_OnCharacterData(xmlpath, xmltag, characters, num_characters); }
	//void xmlReaderOnError(int linenum, int errcode, NXString errstr);

	enum
	{
		DISPATCHABLE_VERSION=0,
	};
private:
	virtual void WASABICALL XMLCallback_OnStartElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, ifc_xmlattributes *attributes){}
	virtual void WASABICALL XMLCallback_OnEndElement(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag) {}
	virtual void WASABICALL XMLCallback_OnCharacterData(const nsxml_char_t *xmlpath, const nsxml_char_t *xmltag, const nsxml_char_t *characters, size_t num_characters){}
};

