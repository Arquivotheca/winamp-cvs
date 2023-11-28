#include "XMLAttributes.h"
#include <wchar.h>


XMLAttributes::XMLAttributes(const wchar_t **_parameters) : parameters(_parameters), num_parameters(0), num_parameters_calculated(false)
{
}

void XMLAttributes::CountTo(size_t x)
{
	if (num_parameters_calculated || x < num_parameters)
		return;

	while (1)
	{
		if (parameters[num_parameters*2] == 0)
		{
			num_parameters_calculated=true;
			return;
		}
		num_parameters++;
		if (num_parameters == x)
			return;
	}
}

void XMLAttributes::Count()
{
	if (num_parameters_calculated)
		return;

	while (1)
	{
		if (parameters[num_parameters*2] == 0)
		{
			num_parameters_calculated=true;
			return;
		}
		num_parameters++;
	}
}

const wchar_t *XMLAttributes::XMLAttributes_GetAttribute(nx_string_t name)
{
	size_t i=0;
	while(1)
	{
		CountTo(i+1);
		if (i<num_parameters)
		{
			if (!wcscmp(name->string, parameters[i*2]))
				return parameters[i*2+1];
		}
		else
			return 0;
		i++;
	};
}

#if 0
const wchar_t *XMLParameters::GetItemName(int i)
{
	CountTo(i);
	if (i < num_parameters)
		return parameters[i*2];
	else
		return 0;
}

const wchar_t *XMLParameters::GetItemValueIndex(int i)
{
	CountTo(i);
	if (i < num_parameters)
		return parameters[i*2+1];
	else
		return 0;
}

int XMLParameters::GetNumItems()
{
	Count();
	return num_parameters;
}

const wchar_t *XMLParameters::GetItemValue(const wchar_t *name)
{
	int i=0;
	while(1)
	{
		CountTo(i+1);
		if (i<num_parameters)
		{
			if (!_wcsicmp(name, parameters[i*2]))
				return parameters[i*2+1];
		}
		else
			return 0;
		i++;
	};
}

int XMLParameters::GetItemValueInt(const wchar_t *name, int def)
{
	const wchar_t *val = GetItemValue(name);
	if (val && val[0])
	{
		int r = wcstol(val, 0, 10);
		return r;
	}
	else
	{
		return def;
	}
}

const wchar_t *XMLParameters::EnumItemValues(const wchar_t *name, int nb)
{
	int i=0; 
	while(1)
	{
		CountTo(i+1);
		if (i<num_parameters)
		{
			if (!_wcsicmp(name, parameters[i*2]) && nb--)
				return parameters[i*2+1];
		}
		else
			return 0;
		i++;
	};
}


#endif