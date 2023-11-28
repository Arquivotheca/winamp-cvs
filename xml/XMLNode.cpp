#include "XMLNode.h"

static int CompareStuff(const wchar_t *const &str1, const wchar_t *const &str2)
{
	return CompareStringW(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT), 0, str1, -1, str2, -1)-2;
}

XMLNode::XMLNode() : content(0), parent(0)
{
}

XMLNode::~XMLNode()
{
	for (PropMap::iterator mapItr=properties.begin();mapItr != properties.end();mapItr++)
	{
		free((wchar_t *)mapItr->first);
		free(mapItr->second);
	}

	for (NodeMap::iterator mapItr=nodes.begin();mapItr != nodes.end();mapItr++)
	{
		NodeList * const nodeList = mapItr->second;
		if (nodeList)
		{
			for (NodeList::iterator itr=nodeList->begin(); itr!= nodeList->end(); itr++)
			{
				delete static_cast<XMLNode *>(*itr);
			}
		}
		free((wchar_t *)mapItr->first);
	}

	free(content);
}

const XMLNode *XMLNode::Get(const wchar_t *tagName) const
{
	NodeMap::iterator itr = nodes.find(tagName);
	if (itr == nodes.end())
		return 0;
	else
	{
		NodeList *list = itr->second;
		return list->at(0);
	}
}

const XMLNode::NodeList *XMLNode::GetList(const wchar_t *tagName) const
{
	NodeMap::iterator itr = nodes.find(tagName);
	if (itr == nodes.end())
	{
		return 0;
	}
	else
	{
		NodeList *list = itr->second;
		return list;
	}
}

const bool XMLNode::Present(const wchar_t *tagName) const 
{
	return nodes.find(tagName) != nodes.end();
}

void XMLNode::SetProperty(const wchar_t *prop, const wchar_t *value)
{
	PropMap::MapPair &pair = properties.getItem(prop);
	if (pair.first == prop) // replace with a copy if we made a new entry
		pair.first = _wcsdup(prop);
	free(pair.second);
	pair.second = _wcsdup(value);
}

void XMLNode::SetContent_Own(wchar_t *new_content)
{
	free(content);
	content = new_content;
}

void XMLNode::AppendContent(wchar_t *append)
{
	if (append)
	{
		if (content)
		{
			size_t old_len = content?wcslen(content):0;
			size_t new_len = wcslen(append)+old_len;
			content = (wchar_t *)realloc(content, (new_len+1)*sizeof(wchar_t));
			wcscat(content, append);
		}
		else
		{
			content=_wcsdup(append);
		}
	}
}

void XMLNode::AddNode(const wchar_t *name, XMLNode *new_node)
{
	// first, add entry in nodes map
	NodeMap::MapPair &pair = nodes.getItem(name);
	if (pair.first == name) // replace with a copy if we made a new entry
		pair.first = _wcsdup(name);

	// make the node list if we need it
	if (!pair.second)
		pair.second = new NodeList;

	pair.second->push_back(new_node);
}

const wchar_t *XMLNode::GetContent() const
{
	return content;
}

const wchar_t *XMLNode::GetProperty(const wchar_t *prop) const
{
	for (PropMap::iterator mapItr=properties.begin();mapItr != properties.end();mapItr++)
	{
		if (CompareStuff(mapItr->first, prop) == 0)
		{
			return mapItr->second;
		}
	}
	return 0;
}
