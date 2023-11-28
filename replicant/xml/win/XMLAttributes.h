#pragma once
#include "xml/ifc_xmlattributes.h"

class XMLAttributes : public ifc_xmlattributes
{
public:
	XMLAttributes(const wchar_t **_parameters);

private:
	/* ifc_xmlattributes implementation */
	const nsxml_char_t *WASABICALL XMLAttributes_GetAttribute(nx_string_t name);

	const wchar_t **parameters;
	size_t num_parameters;
	bool num_parameters_calculated;

	void CountTo(size_t x);
	void Count();
};

