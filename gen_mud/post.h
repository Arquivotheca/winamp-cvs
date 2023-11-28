#pragma once
#include "../xml/obj_xml.h"

/* POSTs data and feeds the return value to your passed-in XML parser */
int PostXML(const char *url, const char *post_data, obj_xml *parser);