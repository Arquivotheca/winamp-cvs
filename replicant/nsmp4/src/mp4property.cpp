/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is MPEG4IP.
 * 
 * The Initial Developer of the Original Code is Cisco Systems Inc.
 * Portions created by Cisco Systems Inc. are
 * Copyright (C) Cisco Systems Inc. 2001.  All Rights Reserved.
 * 
 * Contributor(s): 
 *		Dave Mackie		dmackie@cisco.com
 */

#include "mp4common.h"
#include "foundation/error.h"

MP4Property::MP4Property(const char* name)
{
	m_name = name;
	m_pParentAtom = NULL;
	m_readOnly = false;
	m_implicit = false;
}

bool MP4Property::FindProperty(const char* name, 
	MP4Property** ppProperty, uint32_t* pIndex) 
{
	if (name == NULL) {
		return false;
	}

	if (!strcasecmp(m_name, name)) {
		if (m_pParentAtom) {
			VERBOSE_FIND(m_pParentAtom->GetFile()->GetVerbosity(),
				printf("FindProperty: matched %s\n", name));
		}

		*ppProperty = this;
		return true;
	}
	return false;
}

// Integer Property

uint64_t MP4IntegerProperty::GetValue(uint32_t index)
{
	switch (this->GetType()) {
	case Integer8Property:
		return ((MP4Integer8Property*)this)->GetValue(index);
	case Integer16Property:
		return ((MP4Integer16Property*)this)->GetValue(index);
	case Integer24Property:
		return ((MP4Integer24Property*)this)->GetValue(index);
	case Integer32Property:
		return ((MP4Integer32Property*)this)->GetValue(index);
	case Integer64Property:
		return ((MP4Integer64Property*)this)->GetValue(index);
	default:
		ASSERT(FALSE);
	}
	return (0);
}

void MP4IntegerProperty::SetValue(uint64_t value, uint32_t index)
{
	switch (this->GetType()) {
	case Integer8Property:
		((MP4Integer8Property*)this)->SetValue(value, index);
		break;
	case Integer16Property:
		((MP4Integer16Property*)this)->SetValue(value, index);
		break;
	case Integer24Property:
		((MP4Integer24Property*)this)->SetValue(value, index);
		break;
	case Integer32Property:
		((MP4Integer32Property*)this)->SetValue(value, index);
		break;
	case Integer64Property:
		((MP4Integer64Property*)this)->SetValue(value, index);
		break;
	default:
		ASSERT(FALSE);
	}
}

void MP4IntegerProperty::InsertValue(uint64_t value, uint32_t index)
{
	switch (this->GetType()) {
	case Integer8Property:
		((MP4Integer8Property*)this)->InsertValue(value, index);
		break;
	case Integer16Property:
		((MP4Integer16Property*)this)->InsertValue(value, index);
		break;
	case Integer24Property:
		((MP4Integer24Property*)this)->InsertValue(value, index);
		break;
	case Integer32Property:
		((MP4Integer32Property*)this)->InsertValue(value, index);
		break;
	case Integer64Property:
		((MP4Integer64Property*)this)->InsertValue(value, index);
		break;
	default:
		ASSERT(FALSE);
	}
}

void MP4IntegerProperty::DeleteValue(uint32_t index)
{
	switch (this->GetType()) {
	case Integer8Property:
		((MP4Integer8Property*)this)->DeleteValue(index);
		break;
	case Integer16Property:
		((MP4Integer16Property*)this)->DeleteValue(index);
		break;
	case Integer24Property:
		((MP4Integer24Property*)this)->DeleteValue(index);
		break;
	case Integer32Property:
		((MP4Integer32Property*)this)->DeleteValue(index);
		break;
	case Integer64Property:
		((MP4Integer64Property*)this)->DeleteValue(index);
		break;
	default:
		ASSERT(FALSE);
	}
}

void MP4IntegerProperty::IncrementValue(int32_t increment, uint32_t index)
{
	SetValue(GetValue() + increment);
}

uint64_t MP4IntegerProperty::ReadInteger(MP4File *pFile, size_t bytes)
{ 
	return pFile->ReadUInt(bytes);
} 

void MP4IntegerProperty::WriteInteger(MP4File *pFile, uint64_t value, size_t bytes)
{ 
	pFile->WriteUInt(value, bytes);
} 

// MP4BitfieldProperty

void MP4BitfieldProperty::Read(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	m_values[index] = pFile->ReadBits(m_numBits);
}

void MP4BitfieldProperty::Write(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	pFile->WriteBits(m_values[index], m_numBits);
}

void MP4BitfieldProperty::Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index)
{
	if (m_implicit && !dumpImplicits) {
		return;
	}
	Indent(pFile, indent);

	uint8_t hexWidth = m_numBits / 4;
	if (hexWidth == 0 || (m_numBits % 4)) {
		hexWidth++;
	}
	if (index != 0) 
	fprintf(pFile,  "%s[%u] = %llu (0x%0*llu) <%u bits>\n", m_name, index, m_values[index], (int)hexWidth, m_values[index], m_numBits);
	else 
	  fprintf(pFile, "%s = %llu (0x%0*llu) <%u bits>\n",  m_name, m_values[index], (int)hexWidth, m_values[index], m_numBits);
	fflush(pFile);
}

// MP4Float32Property

void MP4Float32Property::Read(MP4File* pFile, uint32_t index) 
{
	if (m_implicit) {
		return;
	}
	if (m_useFixed16Format) {
		m_values[index] = pFile->ReadFixed16();
	} else if (m_useFixed32Format) {
		m_values[index] = pFile->ReadFixed32();
	} else {
		m_values[index] = pFile->ReadFloat();
	}
}

void MP4Float32Property::Write(MP4File* pFile, uint32_t index) 
{
	if (m_implicit) {
		return;
	}
	if (m_useFixed16Format) {
		pFile->WriteFixed16(m_values[index]);
	} else if (m_useFixed32Format) {
		pFile->WriteFixed32(m_values[index]);
	} else {
		pFile->WriteFloat(m_values[index]);
	}
}

void MP4Float32Property::Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index)
{
	if (m_implicit && !dumpImplicits) {
		return;
	}
	Indent(pFile, indent);
	if (index != 0) 
  	  fprintf(pFile, "%s[%u] = %f\n", 
		  m_name, index, m_values[index]);
	else
 	  fprintf(pFile, "%s = %f\n", 
		  m_name, m_values[index]);
	fflush(pFile);
}
// MP4StringProperty

MP4StringProperty::MP4StringProperty(const char* name, 	bool useCountedFormat, bool useUnicode)
	: MP4Property(name)
{
	SetCount(1);
	m_values[0] = NULL;
	m_useCountedFormat = useCountedFormat;
	m_useExpandedCount = false;
	m_useUnicode = useUnicode;
	m_fixedLength = 0;	// length not fixed
}

MP4StringProperty::~MP4StringProperty() 
{
	uint32_t count = GetCount();
	for (uint32_t i = 0; i < count; i++) {
		MP4Free(m_values[i]);
	}
}

void MP4StringProperty::SetCount(uint32_t count) 
{
	uint32_t oldCount = m_values.Size();

	m_values.Resize(count);

	for (uint32_t i = oldCount; i < count; i++) {
		m_values[i] = NULL;
	}
}

void MP4StringProperty::SetValue(const char* value, uint32_t index) 
{
	if (m_readOnly) {
		throw new MP4Error(EACCES, "property is read-only", m_name);
	}

	MP4Free(m_values[index]);

	if (m_fixedLength) {
		m_values[index] = (char*)MP4Calloc(m_fixedLength + 1);
		if (value) {
			strncpy(m_values[index], value, m_fixedLength);
		}
	} else {
		if (value) {
			if (m_useUnicode)
					m_values[index] = (char *)MP4Stralloc((const uint16_t *)value);
			else
						m_values[index] = MP4Stralloc(value);
		} else {
			m_values[index] = NULL;
		}
	}
}

void MP4StringProperty::Read(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	if (m_useCountedFormat) {
		m_values[index] = pFile->ReadCountedString(
			(m_useUnicode ? 2 : 1), m_useExpandedCount);
	} else if (m_fixedLength) {
		MP4Free(m_values[index]);
		m_values[index] = (char*)MP4Calloc(m_fixedLength + 1);
		pFile->ReadBytes((uint8_t*)m_values[index], m_fixedLength);
	} else {
		if (m_useUnicode)
		{
			m_values[index] = (char *)pFile->ReadUnicodeString();
		}
		else
		{
			char *str = pFile->ReadString();
			if (str && str[0] == 0xFF && str[1] == 0xFE)
				m_useUnicode  = true;
			if (str && str[0] == 0xFE && str[1] == 0xFF)
				m_useUnicode  = true;
			m_values[index] = str;
		}
	}
}

void MP4StringProperty::Write(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	if (m_useCountedFormat) {
		pFile->WriteCountedString(m_values[index],
			(m_useUnicode ? 2 : 1), m_useExpandedCount);
	} else if (m_fixedLength) {
		pFile->WriteBytes((uint8_t*)m_values[index], m_fixedLength);
	} else {
		if (m_useUnicode)
			pFile->WriteUnicodeString((const uint16_t *)m_values[index]);
		else
			pFile->WriteString(m_values[index]);
	}
}

void MP4StringProperty::Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index)
{
	if (m_implicit && !dumpImplicits) {
		return;
	}
	Indent(pFile, indent);
	char indexd[32];
	if (index != 0) {
	  sprintf(indexd, "[%u]", index);
	} else indexd[0] = '\0';

	if (m_useUnicode) {
	  fprintf(pFile, "%s%s = %ls\n", m_name, indexd, (wchar_t*)m_values[index]);
	} else {
	  fprintf(pFile, "%s%s = %s\n", m_name, indexd, m_values[index]);
	}
	fflush(pFile);
}
// MP4BytesProperty

MP4BytesProperty::MP4BytesProperty(const char* name, uint32_t valueSize,
                                   uint32_t defaultValueSize)
	: MP4Property(name)
{
	SetCount(1);
	m_values[0] = (uint8_t*)MP4Calloc(valueSize);
	m_valueSizes[0] = valueSize;
	m_fixedValueSize = 0;
        m_defaultValueSize = defaultValueSize;
}

MP4BytesProperty::~MP4BytesProperty() 
{
	uint32_t count = GetCount();
	for (uint32_t i = 0; i < count; i++) {
		MP4Free(m_values[i]);
	}
}
  
void MP4BytesProperty::SetCount(uint32_t count) 
{
	uint32_t oldCount = m_values.Size();

	m_values.Resize(count);
	m_valueSizes.Resize(count);

	for (uint32_t i = oldCount; i < count; i++) {
		m_values[i] = NULL;
		m_valueSizes[i] = m_defaultValueSize;
	}
}

void MP4BytesProperty::SetValue(const uint8_t* pValue, uint32_t valueSize, uint32_t index) 
{
	if (m_readOnly) {
		throw new MP4Error(EACCES, "property is read-only", m_name);
	}
	if (m_fixedValueSize) {
		if (valueSize > m_fixedValueSize) {
			throw new MP4Error("%s.%s value size %d exceeds fixed value size %d",
					   "MP4BytesProperty::SetValue",
					   GetParentAtom()->GetType(),
					   GetName(),
					   valueSize, 
					   m_fixedValueSize);
		}
		if (m_values[index] == NULL) {
			m_values[index] = (uint8_t*)MP4Calloc(m_fixedValueSize);
			m_valueSizes[index] = m_fixedValueSize;
		}
		if (pValue) {
			memcpy(m_values[index], pValue, valueSize);
		}
	} else {
		MP4Free(m_values[index]);
		if (pValue) {
			m_values[index] = (uint8_t*)MP4Malloc(valueSize);
			memcpy(m_values[index], pValue, valueSize);
			m_valueSizes[index] = valueSize;
		} else {
			m_values[index] = NULL;
			m_valueSizes[index] = 0;
		}
	}
}

int MP4BytesProperty::ModifyPointer(uint8_t **pValue, uint32_t valueSize, uint32_t index)
{
	if (m_readOnly) 
	{
		return NErr_ReadOnly;
	}

	if (m_fixedValueSize) 
	{
		if (valueSize > m_fixedValueSize) 
		{
			return NErr_BadParameter;
		}

		uint8_t *value = m_values[index];
		if (value == NULL) 
		{
			value = (uint8_t*)MP4Calloc(m_fixedValueSize);
			if (!value)
				return NErr_OutOfMemory;

			m_values[index] = value;
			m_valueSizes[index] = m_fixedValueSize;
		}
		*pValue = value;
		return NErr_Success;
	} 
	else 
	{
		MP4Free(m_values[index]);
		uint8_t *value = (uint8_t*)MP4Malloc(valueSize);
		if (!value)
			return NErr_OutOfMemory;

		m_values[index] = value;
		m_valueSizes[index] = valueSize;
		*pValue = value;
		return NErr_Success;
	}
}

void MP4BytesProperty::SetValueSize(uint32_t valueSize, uint32_t index) 
{
	if (m_fixedValueSize) {
		throw new MP4Error("can't change size of fixed sized property",
			"MP4BytesProperty::SetValueSize");
	}
	if (m_values[index] != NULL) {
		m_values[index] = (uint8_t*)MP4Realloc(m_values[index], valueSize);
	}
	m_valueSizes[index] = valueSize;
}

void MP4BytesProperty::SetFixedSize(uint32_t fixedSize) 
{
	m_fixedValueSize = 0;
	for (uint32_t i = 0; i < GetCount(); i++) {
		SetValueSize(fixedSize, i);
	}
	m_fixedValueSize = fixedSize;
}

void MP4BytesProperty::Read(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	MP4Free(m_values[index]);
	m_values[index] = (uint8_t*)MP4Malloc(m_valueSizes[index]);
	pFile->ReadBytes(m_values[index], m_valueSizes[index]);
}

void MP4BytesProperty::Write(MP4File* pFile, uint32_t index)
{
	if (m_implicit) {
		return;
	}
	pFile->WriteBytes(m_values[index], m_valueSizes[index]);
}

void MP4BytesProperty::Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index)
{
	if (m_implicit && !dumpImplicits) {
		return;
	}
	Indent(pFile, indent);
	fputs(m_name, pFile);
	if (index != 0) fprintf(pFile, "[%u]", index);
	fprintf(pFile, " = <%u bytes> ", m_valueSizes[index]);
	for (uint32_t i = 0; i < m_valueSizes[index]; i++) {
		if ((i % 16) == 0 && m_valueSizes[index] > 16) {
			fprintf(pFile, "\n");
			Indent(pFile, indent);
		}
		fprintf(pFile, "%02x ", m_values[index][i]);
	}
	fprintf(pFile, "\n");
	fflush(pFile);
}

// MP4TableProperty

MP4TableProperty::MP4TableProperty(const char* name, MP4IntegerProperty* pCountProperty)
	: MP4Property(name) 
{
	m_pCountProperty = pCountProperty;
	m_pCountProperty->SetReadOnly();
}

MP4TableProperty::~MP4TableProperty()
{
	for (uint32_t i = 0; i < m_pProperties.Size(); i++) {
		delete m_pProperties[i];
	}
}

void MP4TableProperty::AddProperty(MP4Property* pProperty) 
{
	ASSERT(pProperty);
	ASSERT(pProperty->GetType() != TableProperty);
	ASSERT(pProperty->GetType() != DescriptorProperty);
	m_pProperties.Add(pProperty);
	pProperty->SetParentAtom(m_pParentAtom);
	pProperty->SetCount(0);
}

bool MP4TableProperty::FindProperty(const char *name,
	MP4Property** ppProperty, uint32_t* pIndex)
{
	ASSERT(m_name);

	// check if first component of name matches ourselves
	if (!MP4NameFirstMatches(m_name, name)) {
		return false;
	}

	// check if the specified table entry exists
	uint32_t index;
	bool haveIndex = MP4NameFirstIndex(name, &index);
	if (haveIndex) {
		if (index >= GetCount()) {
			return false;
		}
		if (pIndex) {
			*pIndex = index;
		}
	}

	VERBOSE_FIND(m_pParentAtom->GetFile()->GetVerbosity(),
		printf("FindProperty: matched %s\n", name));

	// get name of table property
	const char *tablePropName = MP4NameAfterFirst(name);
	if (tablePropName == NULL) {
		if (!haveIndex) {
			*ppProperty = this;
			return true;
		}
		return false;
	}

	// check if this table property exists
	return FindContainedProperty(tablePropName, ppProperty, pIndex);
}

bool MP4TableProperty::FindContainedProperty(const char *name,
	MP4Property** ppProperty, uint32_t* pIndex)
{
	uint32_t numProperties = m_pProperties.Size();

	for (uint32_t i = 0; i < numProperties; i++) {
		if (m_pProperties[i]->FindProperty(name, ppProperty, pIndex)) {
			return true;
		}
	}
	return false;
}

void MP4TableProperty::Read(MP4File* pFile, uint32_t index)
{
	ASSERT(index == 0);

	if (m_implicit) {
		return;
	}

	uint32_t numProperties = m_pProperties.Size();

	if (numProperties == 0) {
		WARNING(numProperties == 0);
		return;
	}

	uint32_t numEntries = GetCount();

	/* for each property set size */
	for (uint32_t j = 0; j < numProperties; j++) {
		m_pProperties[j]->SetCount(numEntries);
	}

	for (uint32_t i = 0; i < numEntries; i++) {
		ReadEntry(pFile, i);
	}
}

void MP4TableProperty::ReadEntry(MP4File* pFile, uint32_t index)
{
	for (uint32_t j = 0; j < m_pProperties.Size(); j++) {
		m_pProperties[j]->Read(pFile, index);
	}
}

void MP4TableProperty::Write(MP4File* pFile, uint32_t index)
{
	ASSERT(index == 0);

	if (m_implicit) {
		return;
	}

	uint32_t numProperties = m_pProperties.Size();

	if (numProperties == 0) {
		WARNING(numProperties == 0);
		return;
	}

	uint32_t numEntries = GetCount();

	if (m_pProperties[0]->GetCount() != numEntries) {
	  fprintf(stderr, "%s %s \"%s\"table entries %u doesn't match count %u\n", 
		  GetParentAtom() != NULL ? GetParentAtom()->GetType() : "", 
		  GetName(), m_pProperties[0]->GetName(),
		  m_pProperties[0]->GetCount(), numEntries);

	  ASSERT(m_pProperties[0]->GetCount() == numEntries);
	}

	for (uint32_t i = 0; i < numEntries; i++) {
		WriteEntry(pFile, i);
	}
}

void MP4TableProperty::WriteEntry(MP4File* pFile, uint32_t index)
{
	for (uint32_t j = 0; j < m_pProperties.Size(); j++) {
		m_pProperties[j]->Write(pFile, index);
	}
}
void MP4TableProperty::Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index)
{
	ASSERT(index == 0);

	// implicit tables just can't be dumped
	if (m_implicit) {
		return;
	}

	uint32_t numProperties = m_pProperties.Size();

	if (numProperties == 0) {
		WARNING(numProperties == 0);
		return;
	}

	uint32_t numEntries = GetCount();

	for (uint32_t i = 0; i < numEntries; i++) {
		for (uint32_t j = 0; j < numProperties; j++) {
			m_pProperties[j]->Dump(pFile, indent + 1, dumpImplicits, i);
		}
	}
}
// MP4DescriptorProperty
  
MP4DescriptorProperty::MP4DescriptorProperty(const char* name, 
	uint8_t tagsStart, uint8_t tagsEnd, bool mandatory, bool onlyOne)
	: MP4Property(name) 
{ 
	SetTags(tagsStart, tagsEnd);
	m_sizeLimit = 0;
	m_mandatory = mandatory;
	m_onlyOne = onlyOne;
}

MP4DescriptorProperty::~MP4DescriptorProperty() 
{
	for (uint32_t i = 0; i < m_pDescriptors.Size(); i++) {
		delete m_pDescriptors[i];
	}
}

void MP4DescriptorProperty::SetParentAtom(MP4Atom* pParentAtom) {
	m_pParentAtom = pParentAtom;
	for (uint32_t i = 0; i < m_pDescriptors.Size(); i++) {
		m_pDescriptors[i]->SetParentAtom(pParentAtom);
	}
}

MP4Descriptor* MP4DescriptorProperty::AddDescriptor(uint8_t tag)
{
	// check that tag is in expected range
	ASSERT(tag >= m_tagsStart && tag <= m_tagsEnd);

	MP4Descriptor* pDescriptor = CreateDescriptor(tag);
	ASSERT(pDescriptor);

	m_pDescriptors.Add(pDescriptor);
	pDescriptor->SetParentAtom(m_pParentAtom);

	return pDescriptor;
}

void MP4DescriptorProperty::DeleteDescriptor(uint32_t index)
{
	delete m_pDescriptors[index];
	m_pDescriptors.Delete(index);
}

void MP4DescriptorProperty::Generate()
{
	// generate a default descriptor
	// if it is mandatory, and single
	if (m_mandatory && m_onlyOne) 
	{
		MP4Descriptor* pDescriptor = AddDescriptor(m_tagsStart);
		pDescriptor->Generate();
	}
}

bool MP4DescriptorProperty::FindProperty(const char *name,
	MP4Property** ppProperty, uint32_t* pIndex)
{
	// we're unnamed, so just check contained properties
	if (m_name == NULL || !strcmp(m_name, "")) {
		return FindContainedProperty(name, ppProperty, pIndex);
	}

	// check if first component of name matches ourselves
	if (!MP4NameFirstMatches(m_name, name)) {
		return false;
	}

	// check if the specific descriptor entry exists
	uint32_t descrIndex;
	bool haveDescrIndex = MP4NameFirstIndex(name, &descrIndex);

	if (haveDescrIndex && descrIndex >= GetCount()) {
		return false;
	}

	if (m_pParentAtom) {
		VERBOSE_FIND(m_pParentAtom->GetFile()->GetVerbosity(),
			printf("FindProperty: matched %s\n", name));
	}

	// get name of descriptor property
	name = MP4NameAfterFirst(name);
	if (name == NULL) {
		if (!haveDescrIndex) {
			*ppProperty = this;
			return true;
		}
		return false;
	}

	/* check rest of name */
	if (haveDescrIndex) {
		return m_pDescriptors[descrIndex]->FindProperty(name, 
			ppProperty, pIndex); 
	} else {
		return FindContainedProperty(name, ppProperty, pIndex);
	}
}

bool MP4DescriptorProperty::FindContainedProperty(const char *name,
	MP4Property** ppProperty, uint32_t* pIndex)
{
	for (uint32_t i = 0; i < m_pDescriptors.Size(); i++) {
		if (m_pDescriptors[i]->FindProperty(name, ppProperty, pIndex)) {
			return true;
		}
	}
	return false;
}

void MP4DescriptorProperty::Read(MP4File* pFile, uint32_t index)
{
	ASSERT(index == 0);

	if (m_implicit) {
		return;
	}

	uint64_t start = pFile->GetPosition();

	while (true) {
		// enforce size limitation
		if (m_sizeLimit && pFile->GetPosition() >= start + m_sizeLimit) {
			break;
		}

		uint8_t tag;
		try {
			pFile->PeekByte(&tag);
		}
		catch (MP4Error* e) {
			if (pFile->GetPosition() >= pFile->GetSize()) {
				// EOF
				delete e;
				break;
			}
			throw e;
		}

		// check if tag is in desired range
		if (tag < m_tagsStart || tag > m_tagsEnd) {
			break;
		}

		MP4Descriptor* pDescriptor = 
			AddDescriptor(tag);

		pDescriptor->Read(pFile);
	}
}

void MP4DescriptorProperty::Write(MP4File* pFile, uint32_t index)
{
	ASSERT(index == 0);

	if (m_implicit) {
		return;
	}

	for (uint32_t i = 0; i < m_pDescriptors.Size(); i++) {
		m_pDescriptors[i]->Write(pFile);
	}
}

void MP4DescriptorProperty::Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index)
{
	ASSERT(index == 0);

	if (m_implicit && !dumpImplicits) {
		return;
	}

	if (m_name) {
		Indent(pFile, indent);
		if (index != 0) 
		  fprintf(pFile, "%s[%u]\n", m_name, index);
		else
		  fprintf(pFile, "%s\n", m_name);
		indent++;
	}

	for (uint32_t i = 0; i < m_pDescriptors.Size(); i++) {
		m_pDescriptors[i]->Dump(pFile, indent, dumpImplicits);
	}
}
