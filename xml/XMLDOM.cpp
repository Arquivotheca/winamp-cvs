#include "XMLDOM.h"

XMLDOM::XMLDOM() : curtext(0), curNode(0) 
{
	xmlNode = new XMLNode;
	curNode = xmlNode;
	curtext_len = 0;
}

XMLDOM::~XMLDOM()
{
	free(curtext);
	delete xmlNode;
}

void XMLDOM::StartTag(const wchar_t *xmlpath, const wchar_t *xmltag, ifc_xmlreaderparams *params)
{
	XMLNode *newNode = new XMLNode;

	int num = params->getNbItems();
	for (int i = 0;i < num;i++)
		newNode->SetProperty(params->getItemName(i), params->getItemValue(i));

	newNode->parent = curNode;
	curNode->SetContent_Own(curtext);
	curtext = 0;

	curNode->AddNode(xmltag, newNode);
	curNode = newNode;
}

void XMLDOM::EndTag(const wchar_t *xmlpath, const wchar_t *xmltag)
{
	curNode->AppendContent(curtext);

	free(curtext);
	curtext=0;

	curNode = curNode->parent;
}

void XMLDOM::TextHandler(const wchar_t *xmlpath, const wchar_t *xmltag, const wchar_t *str)
{
	if (str)
	{
		if (curtext)
		{
			size_t new_len = wcslen(str)+curtext_len;
			curtext = (wchar_t *)realloc(curtext, (new_len+1)*sizeof(wchar_t));
			if (curtext)
				wcscpy(curtext+curtext_len, str);
			curtext_len = new_len;
		}
		else
		{
			curtext_len = wcslen(str);
			curtext = (wchar_t *)malloc((curtext_len+1)*sizeof(wchar_t));
			if (curtext)
				memcpy(curtext, str, (curtext_len+1)*sizeof(wchar_t));
		}
	}
}

const XMLNode *XMLDOM::GetRoot() const
{
	return xmlNode;
}

#define CBCLASS XMLDOM
START_DISPATCH;
VCB(ONSTARTELEMENT, StartTag)
VCB(ONENDELEMENT, EndTag)
VCB(ONCHARDATA, TextHandler)
END_DISPATCH;
#undef CBCLASS