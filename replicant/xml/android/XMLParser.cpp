#include "XMLParser.h"
#include "xml/ifc_xmlcallback.h"
#include "regexp.h"
#include "XMLAttributes.h"
#include "nu/strsafe.h"

static bool DoMatch(nx_string_t match_string, const char *string, int match_style)
{
	switch(match_style)
	{
	case obj_xml::MATCH_EXACT:
		return !strcmp(match_string->string, string);
	case obj_xml::MATCH_REGEX:
		return Match(match_string->string, string);
	}
	return false;
}

CallbackStruct::CallbackStruct(ifc_xmlcallback *callback, nx_string_t _match, int match_style) : callback(callback), match_style(match_style)
{
	match = NXStringRetain(_match);
	if (callback)
		callback->Retain();
}

CallbackStruct::CallbackStruct() : callback(0), match(0)
{}

CallbackStruct::~CallbackStruct()
{
	NXStringRelease(match);
	if (callback)
		callback->Release();
}

/* --- */

void XMLCALL DStartTag(void *data, const XML_Char *name, const XML_Char **atts) {	((XMLParser *)data)->StartTag(name, atts); }
void XMLCALL DEndTag(void *data, const XML_Char *name) { ((XMLParser *)data)->EndTag(name); }
void XMLCALL DTextHandler(void *data, const XML_Char *s, int len) { ((XMLParser *)data)->TextHandler(s, len); }

int XMLCALL UnknownEncoding(void *data, const XML_Char *name, XML_Encoding *info);

XMLParser::XMLParser() : parser(0), path_size(0)
{
}


XMLParser::~XMLParser()
{
	if (parser)
		XML_ParserFree(parser);
	callbacks.deleteAll();	
}

const char *XMLParser::AddPath(const char *node)
{
	current_node.CopyFrom(node);
	size_t addLength = strlen(node);
	path_size += addLength + 1;

	path_string.Grow(path_size, QDString::RETAIN_DATA);

	char *p = path_string;
	size_t size = path_size;

	if (p[0])
		StringCchCatExA(p, size, "\f", &p, &size, 0);
	StringCchCatA(p, size, node);


	return path_string;
}

const char *XMLParser::BuildPath()
{
	return path_string;
}

const char *XMLParser::RemovePath(const char *node)
{
	size_t removeLength = strlen(node);
	size_t trimLocation = path_size - removeLength;
	trimLocation--;
	if (trimLocation != 0)
		trimLocation--;
	path_string[trimLocation]=0;

	path_size=trimLocation+1;
	if (path_size == 1)
		path_size=0;

	if (path_string[0])
	{
		const char *last_node = strrchr(path_string, '\f');
		if (last_node)
		{
			current_node.CopyFrom(last_node+1);
		}
		else
			current_node.CopyFrom(path_string);
	}
	else
		current_node.CopyFrom("");

	return path_string;
}

/* --------------------- obj_xml implementations --------------------- */
void XMLParser::XML_RegisterCallback(nx_string_t match_string, int match_style, ifc_xmlcallback *callback)
{
	callbacks.push_back(new CallbackStruct(callback, match_string, match_style));
}

void XMLParser::XML_UnregisterCallback(ifc_xmlcallback *callback)
{
	for (CallbackList::iterator itr=callbacks.begin();itr!=callbacks.end();itr++)
	{
		CallbackStruct *callback_itr = *itr;
		if (callback_itr && callback_itr->callback == callback)
		{
			callbacks.erase(callback_itr);
			delete callback_itr;
			return;
		}
	}
}

int XMLParser::XML_Feed(const void *data, size_t dataSize)
{
	XML_Status error;

	while (dataSize >= 0x7FFFFFFFU) // handle really really big data sizes (hopefully this won't happen)
	{
		XML_Parse(parser, reinterpret_cast<const char *>(data), 0x7FFFFFFF, 0);
		dataSize -= 0x7FFFFFFFU;
	}
	error = XML_Parse(parser, reinterpret_cast<const char *>(data), static_cast<int>(dataSize), 0);

	if (error == XML_STATUS_ERROR)
	{
		// TODO: set a flag to prevent further parsing until a Reset occurs
		XML_Error errorCode = XML_GetErrorCode(parser);
		int line = XML_GetCurrentLineNumber(parser);
		// TODO: int column = XML_GetCurrentColumnNumber(parser);
		const char *errorString = XML_ErrorString(errorCode);

		for (int i = 0;i != callbacks.size();i++)
		{
#if 0 // TODO
			if (NULL != callbacks[i])
			{
				nx_string_t nx_error_string = NXStringCreate(errorString);
				callbacks[i]->callback->xmlReaderOnError(line, errorCode, nx_error_string);
				NXStringRelease(nx_error_string);
			}
#endif
		}
		return NErr_Error;
	}

	return NErr_Success;
}

int XMLParser::XML_Flush()
{
	// passing this sequence tells expat that we're done
	if (XML_Parse(parser, 0, 0, 1) == XML_STATUS_ERROR)
		return NErr_Error;
	return NErr_Success;
}

void XMLParser::XML_Close()
{
	if (parser)
		XML_ParserFree(parser);
	parser = 0;
}

void XMLParser::XML_PushContext()
{
	context.push_back((nu::PtrDequeNode *)parser);
	parser = XML_ExternalEntityParserCreate(parser, "\0", NULL);
}

void XMLParser::XML_PopContext()
{
	if (parser)
		XML_ParserFree(parser);
	parser = (XML_Parser)context.back();
	XML_SetUserData(parser, this); // restore object pointer
	context.pop_back();
}

int XMLParser::XML_Open()
{
	parser = XML_ParserCreate(0); // create the expat parser
	if (!parser)
		return NErr_Error;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return NErr_Success;
}

int XMLParser::XML_OpenNamespace()
{
	parser = XML_ParserCreateNS(0, L'#'); // create the expat parser, using # to separate namespace URI from element name
	if (!parser)
		return NErr_Error;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return NErr_Success;
}

void *XMLParser::XML_GetBuffer(size_t bytes)
{
	return ::XML_GetBuffer(parser, static_cast<int>(bytes));
}

int XMLParser::XML_ParseBuffer(size_t bytes)
{
	XML_Status error;

	error = ::XML_ParseBuffer(parser, static_cast<int>(bytes), 0);

	if (error == XML_STATUS_ERROR)
	{
		// TODO: set a flag to prevent further parsing until a Reset occurs
		XML_Error errorCode = XML_GetErrorCode(parser);
		int line = XML_GetCurrentLineNumber(parser);
		// TODO: int column = XML_GetCurrentColumnNumber(parser);
		const char *errorString = XML_ErrorString(errorCode);

		for (int i = 0;i != callbacks.size();i++)
		{
#if 0 // TODO
			if (NULL != callbacks[i])
			{
				nx_string_t nx_error_string = NXStringCreate(errorString);
				callbacks[i]->callback->xmlReaderOnError(line, errorCode, nx_error_string);
				NXStringRelease(nx_error_string);
			}
#endif
		}
		return NErr_Error;
	}

	return NErr_Success;
}

void XMLParser::XML_Reset()
{
	if (parser)
	{
		XML_ParserReset(parser, 0);
		XML_SetUserData(parser, this); // give our object pointer as context
		XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
		XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	}
}

/* Expat callbacks */
void XMLParser::StartTag(const XML_Char *name, const XML_Char **atts)
{
	const char *xmlpath = AddPath(name);
	XMLAttributes xml_attributes(atts);

	CallbackList::iterator itr=callbacks.begin();
	while (itr != callbacks.end())
	{
		CallbackStruct *callback = *itr;
		itr++; // iterate now because XML_UnregisterCallback could be called during callback
		if (callback && DoMatch(callback->match, xmlpath, callback->match_style))
		{
			callback->callback->OnStartElement(xmlpath, name, &xml_attributes);
		}
	}
}

void XMLParser::EndTag(const XML_Char *name)
{
	end_path_string.CopyFrom(BuildPath());

	RemovePath(name);

	CallbackList::iterator itr=callbacks.begin();
	while (itr != callbacks.end())
	{
		CallbackStruct *callback = *itr;
		itr++; // iterate now because XML_UnregisterCallback could be called during callback
		if (callback && DoMatch(callback->match, end_path_string, callback->match_style))
		{
			callback->callback->OnEndElement(end_path_string, name);
		}
	}
}

void XMLParser::TextHandler(const XML_Char *s, int len)
{
	if (len)
	{
		const char *xmlpath = BuildPath();

		CallbackList::iterator itr=callbacks.begin();
		while (itr != callbacks.end())
		{
			CallbackStruct *callback = *itr;
			itr++; // iterate now because XML_UnregisterCallback could be called during callback
			if (callback && DoMatch(callback->match, xmlpath, callback->match_style))
			{
				callback->callback->OnCharacterData(xmlpath, current_node, s, (size_t)len);
			}
		}
	}
}


#if 0
#include <memory.h>

#include <strsafe.h>
#include <wctype.h>







void XMLReader::SetEncoding(const char *encoding)
{
	XML_SetEncoding(parser, encoding);
}
#endif
/* quick & dirty string */

QDString::QDString() : str(0), len(0)
{}

QDString::~QDString()
{
	free(str);
}

QDString::operator char *()
{
	return str;
}

void QDString::Grow(size_t newLen, CopyData copyData)
{
	if (newLen > len)
	{
		char *newStr = (char *)malloc(newLen * sizeof(char));
		newStr[0]=0;
		if (copyData == RETAIN_DATA && str)
			memcpy(newStr, str, len*sizeof(char)); // safe because newLen > len

		free(str);
		str=newStr;
		len = newLen;
	}
}

void QDString::CopyFrom(const char *intake)
{
	size_t len = strlen(intake)+1;
	Grow(len, QDString::DESTROY_DATA);
	StringCchCopyA(str, len, intake);
}

