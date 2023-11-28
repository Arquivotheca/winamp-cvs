#include "XMLReader.h"
#include "ifc_xmlreadercallback.h"
#include "XMLParameters.h"
#include <memory.h>
#include "../nu/regexp.h"
#include "../nu/strsafe.h"
#include <wctype.h>

/* TODO:
try to remove CharUpper (but towupper doesn't deal with non-english very well)
*/

#ifdef __APPLE__
void CharUpper(wchar_t *src)
{
  while (*src)
  {
    *src = (wint_t)towupper(*src);
    src++;
  }
}

wchar_t *_wcsdup(const wchar_t *src)
{
  if (!src)
    return 0;
  size_t len = wcslen(src)+1;
  if (len) // check for integer wraparound
  {
    wchar_t *newstr = (wchar_t *)malloc(sizeof(wchar_t)*len);
    wcscpy(newstr, src);
    return newstr;
  }
  return 0;
}
#endif

CallbackStruct::CallbackStruct(ifc_xmlreadercallback *_callback, const wchar_t *_match, bool doUpper)
{
	match = _wcsdup(_match);
	if (doUpper)
		CharUpper(match);
	callback = _callback;
}

CallbackStruct::CallbackStruct() : callback(0), match(0)
{}

CallbackStruct::~CallbackStruct()
{
	free(match);
}

/* --- */

void XMLCALL DStartTag(void *data, const XML_Char *name, const XML_Char **atts) {	((XMLReader *)data)->StartTag(name, atts); }
void XMLCALL DEndTag(void *data, const XML_Char *name) { ((XMLReader *)data)->EndTag(name); }
void XMLCALL DTextHandler(void *data, const XML_Char *s, int len) { ((XMLReader *)data)->TextHandler(s, len); }

int XMLCALL UnknownEncoding(void *data, const XML_Char *name, XML_Encoding *info);

XMLReader::XMLReader()
: parser(0), pathSize(0)
{
	case_sensitive=false;
}

XMLReader::~XMLReader()
{
	for (size_t i = 0;i != callbacks.size(); i++)
	{
		delete callbacks[i];
		callbacks[i]=0;
	}
}

void XMLReader::RegisterCallback(const wchar_t *matchstr, ifc_xmlreadercallback *callback)
{
	callbacks.push_back(new CallbackStruct(callback, matchstr, !case_sensitive));
}

void XMLReader::UnregisterCallback(ifc_xmlreadercallback *callback)
{
	for (size_t i = 0;i != callbacks.size(); i++)
	{
		if (callbacks[i] && callbacks[i]->callback == callback)
		{
			delete callbacks[i];
			callbacks[i]=0; // we set it to 0 so this can be called during a callback
		}
	}
}

int XMLReader::Open()
{
	parser = XML_ParserCreate(0); // create the expat parser
	if (!parser)
		return OBJ_XML_FAILURE;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return OBJ_XML_SUCCESS;
}

int XMLReader::OpenNamespace()
{
	parser = XML_ParserCreateNS(0, L'#'); // create the expat parser, using # to separate namespace URI from element name
	if (!parser)
		return OBJ_XML_FAILURE;

	XML_SetUserData(parser, this); // give our object pointer as context
	XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
	XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	XML_SetUnknownEncodingHandler(parser, UnknownEncoding, 0); // setup the character set encoding stuff

	return OBJ_XML_SUCCESS;
}

void XMLReader::OldFeed(void *data, size_t dataSize)
{
	Feed(data, dataSize);
}

int XMLReader::Feed(void *data, size_t dataSize)
{
	XML_Status error;
	if (data && dataSize)
	{
		while (dataSize >= 0x7FFFFFFFU) // handle really really big data sizes (hopefully this won't happen)
		{
			XML_Parse(parser, reinterpret_cast<const char *>(data), 0x7FFFFFFF, 0);
			dataSize -= 0x7FFFFFFFU;
		}
		error = XML_Parse(parser, reinterpret_cast<const char *>(data), static_cast<int>(dataSize), 0);
	}
	else
		error = XML_Parse(parser, 0, 0, 1); // passing this sequence tells expat that we're done

	if (error == XML_STATUS_ERROR)
	{
		// TODO: set a flag to prevent further parsing until a Reset occurs
		XML_Error errorCode = XML_GetErrorCode(parser);
		int line = XML_GetCurrentLineNumber(parser);
		// TODO: int column = XML_GetCurrentColumnNumber(parser);
		const wchar_t *errorString = XML_ErrorString(errorCode);

		for (int i = 0;i != callbacks.size();i++)
		{
			if (NULL != callbacks[i])
			{
				callbacks[i]->callback->xmlReaderOnError(line, errorCode, errorString);
			}
		}
		return OBJ_XML_FAILURE;
	}

	return OBJ_XML_SUCCESS;
}

void XMLReader::Close()
{
	if (parser)
		XML_ParserFree(parser);
	parser = 0;
}

const wchar_t *XMLReader::BuildPath()
{
	return pathString;
}

const wchar_t *XMLReader::AddPath(const wchar_t *node)
{
	currentNode.CopyFrom(node);
	size_t addLength = wcslen(node);
	pathSize += addLength + 1;

	pathString.Grow(pathSize, QDString::RETAIN_DATA);

	wchar_t *p = pathString;
	size_t size = pathSize;

	if (p[0])
		StringCchCatExW(p, size, L"\f", &p, &size, 0);
	StringCchCatW(p, size, node);

	if (!case_sensitive)
		CharUpper(pathString); // TODO: can't we just do CharUpper(p)?

	return pathString;
}

const wchar_t *XMLReader::RemovePath(const wchar_t *node)
{
	size_t removeLength = wcslen(node);
	size_t trimLocation = pathSize - removeLength;
	trimLocation--;
	if (trimLocation != 0)
		trimLocation--;
	pathString[trimLocation]=0;

	pathSize=trimLocation+1;
	if (pathSize == 1)
		pathSize=0;

	if (pathString[0])
	{
		const wchar_t *last_node = wcsrchr(pathString, '\f');
		if (last_node)
		{
			currentNode.CopyFrom(last_node+1);
		}
		else
			currentNode.CopyFrom(pathString);
	}
	else
			currentNode.CopyFrom(L"");

	return pathString;
}

void XMLCALL XMLReader::StartTag(const XML_Char *name, const XML_Char **atts)
{
	const wchar_t *xmlpath = AddPath(name);

	XMLParameters xmlParameters(atts);
	for (size_t i = 0;i != callbacks.size();i++)
	{
		if (callbacks[i] && Match(callbacks[i]->match, xmlpath))
			callbacks[i]->callback->xmlReaderOnStartElementCallback(xmlpath, name, static_cast<ifc_xmlreaderparams *>(&xmlParameters));
	}
}

void XMLCALL XMLReader::EndTag(const XML_Char *name)
{
	endPathString.CopyFrom(BuildPath());

	RemovePath(name);

	for (size_t i = 0;i != callbacks.size();i++)
	{
		if (callbacks[i] && Match(callbacks[i]->match, endPathString))
			callbacks[i]->callback->xmlReaderOnEndElementCallback(endPathString, name);
	}
}

void XMLCALL XMLReader::TextHandler(const XML_Char *s, int len)
{
	if (len)
	{
		textCache.Grow(len+1, QDString::DESTROY_DATA);
		memcpy((wchar_t *)textCache, s, len*sizeof(wchar_t));
		textCache[len] = 0;

		const wchar_t *xmlpath = BuildPath();

		for (size_t i = 0;i != callbacks.size();i++)
		{
			if (callbacks[i] && Match(callbacks[i]->match, xmlpath))
				callbacks[i]->callback->xmlReaderOnCharacterDataCallback(xmlpath, currentNode, textCache);
		}
	}
}

void XMLReader::PushContext()
{
	context.push_back(parser);
	parser = XML_ExternalEntityParserCreate(parser, L"\0", NULL);
}

void XMLReader::PopContext()
{
	if (parser)
		XML_ParserFree(parser);
	parser = context.back();
	context.pop_back();
}

void XMLReader::Reset()
{
	if (parser)
	{
		XML_ParserReset(parser, 0);
		XML_SetUserData(parser, this); // give our object pointer as context
		XML_SetElementHandler(parser, DStartTag, DEndTag); // set the tag callbacks
		XML_SetCharacterDataHandler(parser, DTextHandler); // set the text callbacks
	}
}

void XMLReader::SetEncoding(const wchar_t *encoding)
{
	XML_SetEncoding(parser, encoding);
}

int XMLReader::SetCaseSensitive()
{
	case_sensitive=true;
	return OBJ_XML_SUCCESS;
}

#define CBCLASS XMLReader
START_DISPATCH;
VCB(OBJ_XML_REGISTERCALLBACK, RegisterCallback)
VCB(OBJ_XML_UNREGISTERCALLBACK, UnregisterCallback)
CB(OBJ_XML_OPEN, Open)
CB(OBJ_XML_OPEN2, OpenNamespace)
VCB(OBJ_XML_OLDFEED, OldFeed)
CB(OBJ_XML_FEED, Feed)
VCB(OBJ_XML_CLOSE, Close)
VCB(OBJ_XML_INTERRUPT, PushContext)
VCB(OBJ_XML_RESUME, PopContext)
VCB(OBJ_XML_RESET, Reset)
VCB(OBJ_XML_SETENCODING, SetEncoding)
CB(OBJ_XML_SETCASESENSITIVE, SetCaseSensitive)
END_DISPATCH;
#undef CBCLASS

/* quick & dirty string */

QDString::QDString() : str(0), len(0)
{}

QDString::~QDString()
{
	free(str);
}

QDString::operator wchar_t *()
{
	return str;
}

void QDString::Grow(size_t newLen, CopyData copyData)
{
	if (newLen > len)
	{
		wchar_t *newStr = (wchar_t *)malloc(newLen * sizeof(wchar_t));
		newStr[0]=0;
		if (copyData == RETAIN_DATA && str)
			memcpy(newStr, str, len*sizeof(wchar_t)); // safe because newLen > len

		free(str);
		str=newStr;
		len = newLen;
	}
}

void QDString::CopyFrom(const wchar_t *intake)
{
	size_t len = wcslen(intake)+1;
	Grow(len, QDString::DESTROY_DATA);
	StringCchCopyW(str, len, intake);
}