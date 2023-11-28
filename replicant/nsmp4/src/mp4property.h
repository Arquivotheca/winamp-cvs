#pragma once
// forward declarations
#include "mp4array.h"
class MP4Atom;
class MP4File;

class MP4Descriptor;
typedef MP4TArray<MP4Descriptor *> MP4DescriptorArray;

enum MP4PropertyType {
	Integer8Property,
	Integer16Property,
	Integer24Property,
	Integer32Property,
	Integer64Property,
	Float32Property,
	StringProperty,
	BytesProperty,
	TableProperty,
	DescriptorProperty,
};

class MP4Property {
public:
	MP4Property(const char *name = NULL);

	virtual ~MP4Property() { }

	MP4Atom* GetParentAtom() {
		return m_pParentAtom;
	}
	virtual void SetParentAtom(MP4Atom* pParentAtom) {
		m_pParentAtom = pParentAtom;
	}

	const char *GetName() {
		return m_name;
	}

	virtual MP4PropertyType GetType() = 0; 

	bool IsReadOnly() {
		return m_readOnly;
	}
	void SetReadOnly(bool value = true) {
		m_readOnly = value;
	}

	bool IsImplicit() {
		return m_implicit;
	}
	void SetImplicit(bool value = true) {
		m_implicit = value;
	}

	virtual uint32_t GetCount() = 0;
	virtual void SetCount(uint32_t count) = 0;
	virtual void Generate() { /* default is a no-op */ };
	virtual void Read(MP4File* pFile, uint32_t index = 0) = 0;
	virtual void Write(MP4File* pFile, uint32_t index = 0) = 0;
	virtual void Dump(FILE* pFile, uint8_t indent,	bool dumpImplicits, uint32_t index = 0) = 0;
	virtual bool FindProperty(const char* name,	MP4Property** ppProperty, uint32_t* pIndex = NULL);

protected:
	MP4Atom* m_pParentAtom;
	const char* m_name;
	bool m_readOnly;
	bool m_implicit;
};

typedef MP4TArray<MP4Property *> MP4PropertyArray;

class MP4IntegerProperty : public MP4Property {
protected:
	MP4IntegerProperty(const char *name): MP4Property(name) { }

public:
	uint64_t GetValue(uint32_t index = 0);
	void SetValue(uint64_t value, uint32_t index = 0);
	void InsertValue(uint64_t value, uint32_t index = 0);
	void DeleteValue(uint32_t index = 0);
	void IncrementValue(int32_t increment = 1, uint32_t index = 0);
	uint64_t ReadInteger(MP4File *pFile, size_t bytes);		
	void WriteInteger(MP4File *pFile, uint64_t value, size_t bytes);
};

template <class val_t, uint8_t size, MP4PropertyType prop_type>
class MP4IntegerPropertyT : public MP4IntegerProperty { 
	public: 
		MP4IntegerPropertyT(const char *name) 
			: MP4IntegerProperty(name) { 
			SetCount(1); 
			m_values[0] = 0; 
		} 
		
		MP4PropertyType GetType() { 
			//return Integer##xsize##Property; 
			return prop_type;
		} 
		
		uint32_t GetCount() { 
			return m_values.Size(); 
		} 

		void SetCount(uint32_t count) { 
			m_values.Resize(count); 
		} 
		
		val_t GetValue(uint32_t index = 0) { 
			return m_values[index]; 
		}
		
		void SetValue(val_t value, uint32_t index = 0) 
		{ 
			if (m_readOnly) { 
				throw new MP4Error(EACCES, "property is read-only", m_name); \
			} 
			m_values[index] = value; 
		} 
		void AddValue(val_t value) { 
			m_values.Add(value); 
		} 
		void InsertValue(val_t value, uint32_t index) { 
			m_values.Insert(value, index); 
		} 
		void DeleteValue(uint32_t index) { 
			m_values.Delete(index); 
		} 
		void IncrementValue(int32_t increment = 1, uint32_t index = 0) { 
			m_values[index] += increment; 
		} 
		
		void Read(MP4File *pFile, uint32_t index = 0) { 
			if (m_implicit) { 
				return; 
			} 
			m_values[index] = (val_t)MP4IntegerProperty::ReadInteger(pFile, size/8);
		} 

		void Write(MP4File *pFile, uint32_t index = 0) { 
			if (m_implicit) { 
				return; 
			} 
			MP4IntegerProperty::WriteInteger(pFile, (uint64_t)m_values[index], size/8);
		} 
		void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0)
		{
			if (m_implicit && !dumpImplicits) {
				return;
			}
			Indent(pFile, indent);
			if (index != 0) 
				fprintf(pFile, "%s[%u] = %llu (0x%0*llx)\n",  m_name, index, (uint64_t)m_values[index], size/4, (uint64_t)m_values[index]);
			else
				fprintf(pFile, "%s = %llu (0x%0*llx)\n",  m_name, (uint64_t)m_values[index], size/4,(uint64_t)m_values[index]);
			fflush(pFile);
		}
	protected: 
		MP4TArray<val_t> m_values;
	};

#define MP4INTEGER_PROPERTY_DECL(val_t, xsize)  typedef MP4IntegerPropertyT<val_t, xsize, Integer##xsize##Property> MP4Integer##xsize##Property;

MP4INTEGER_PROPERTY_DECL(uint8_t, 8);
MP4INTEGER_PROPERTY_DECL(uint16_t, 16);
MP4INTEGER_PROPERTY_DECL(uint32_t, 24);
MP4INTEGER_PROPERTY_DECL(uint32_t, 32);
MP4INTEGER_PROPERTY_DECL(uint64_t, 64);

class MP4BitfieldProperty : public MP4Integer64Property {
public:
	MP4BitfieldProperty(const char *name, uint8_t numBits)
		: MP4Integer64Property(name) {
		ASSERT(numBits != 0);
		ASSERT(numBits <= 64);
		m_numBits = numBits;
	}

	uint8_t GetNumBits() {
		return m_numBits;
	}
	void SetNumBits(uint8_t numBits) {
		m_numBits = numBits;
	}

	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);

protected:
	uint8_t m_numBits;
};

class MP4Float32Property : public MP4Property {
public:
	MP4Float32Property(const char *name)
		: MP4Property(name) {
		m_useFixed16Format = false;
		m_useFixed32Format = false;
		SetCount(1);
		m_values[0] = 0.0;
	}

	MP4PropertyType GetType() {
		return Float32Property;
	}

	uint32_t GetCount() {
		return m_values.Size();
	}
	void SetCount(uint32_t count) {
		m_values.Resize(count);
	}

	float GetValue(uint32_t index = 0) {
		return m_values[index];
	}

	void SetValue(float value, uint32_t index = 0) {
		if (m_readOnly) {
			throw new MP4Error(EACCES, "property is read-only", m_name);
		}
		m_values[index] = value;
	}

	void AddValue(float value) {
		m_values.Add(value);
	}

	void InsertValue(float value, uint32_t index) {
		m_values.Insert(value, index);
	}

	bool IsFixed16Format() {
		return m_useFixed16Format;
	}

	void SetFixed16Format(bool useFixed16Format = true) {
		m_useFixed16Format = useFixed16Format;
	}

	bool IsFixed32Format() {
		return m_useFixed32Format;
	}

	void SetFixed32Format(bool useFixed32Format = true) {
		m_useFixed32Format = useFixed32Format;
	}

	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);
protected:
	bool m_useFixed16Format;
	bool m_useFixed32Format;
	MP4Float32Array m_values;
};

class MP4StringProperty : public MP4Property {
public:
	MP4StringProperty(const char *name, 
	  bool useCountedFormat = false, bool useUnicode = false);

	~MP4StringProperty();

	MP4PropertyType GetType() {
		return StringProperty;
	}

	uint32_t GetCount() {
		return m_values.Size();
	}

	void SetCount(uint32_t count);

	const char* GetValue(uint32_t index = 0) {
		return m_values[index];
	}

	void SetValue(const char* value, uint32_t index = 0);

	void AddValue(const char* value) {
		uint32_t count = GetCount();
		SetCount(count + 1); 
		SetValue(value, count);
	}

	bool IsCountedFormat() {
		return m_useCountedFormat;
	}

	void SetCountedFormat(bool useCountedFormat) {
		m_useCountedFormat = useCountedFormat;
	}

	bool IsExpandedCountedFormat() {
		return m_useExpandedCount;
	}

	void SetExpandedCountedFormat(bool useExpandedCount) {
		m_useExpandedCount = useExpandedCount;
	}

	bool IsUnicode() {
		return m_useUnicode;
	}

	void SetUnicode(bool useUnicode) {
		m_useUnicode = useUnicode;
	}

	uint32_t GetFixedLength() {
		return m_fixedLength;
	}

	void SetFixedLength(uint32_t fixedLength) {
		m_fixedLength = fixedLength;
	}

	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);
protected:
	bool m_useCountedFormat;
	bool m_useExpandedCount;
	bool m_useUnicode;
	uint32_t m_fixedLength;

	MP4StringArray m_values;
};

class MP4BytesProperty : public MP4Property {
public:
	MP4BytesProperty(const char *name, uint32_t valueSize = 0,
                         uint32_t defaultValueSize = 0);

	~MP4BytesProperty();

	MP4PropertyType GetType() {
		return BytesProperty;
	}

	uint32_t GetCount() {
		return m_values.Size();
	}

	void SetCount(uint32_t count);

	void GetValue(uint8_t** ppValue, uint32_t* pValueSize, 
	  uint32_t index = 0) {
		// N.B. caller must free memory
		*ppValue = (uint8_t*)MP4Malloc(m_valueSizes[index]);
		memcpy(*ppValue, m_values[index], m_valueSizes[index]);
		*pValueSize = m_valueSizes[index];
	}

	void GetPointer(const uint8_t** ppValue, uint32_t* pValueSize, uint32_t index = 0) 
	{
		*ppValue = m_values[index];
		*pValueSize = m_valueSizes[index];
	}

	void CopyValue(uint8_t* pValue, uint32_t index = 0) {
		// N.B. caller takes responsbility for valid pointer
		// and sufficient memory at the destination
		memcpy(pValue, m_values[index], m_valueSizes[index]);
	}

	void SetValue(const uint8_t* pValue, uint32_t valueSize, uint32_t index = 0);
	int ModifyPointer(uint8_t **pValue, uint32_t valueSize, uint32_t index = 0);

	void AddValue(const uint8_t* pValue, uint32_t valueSize) {
		uint32_t count = GetCount();
		SetCount(count + 1); 
		SetValue(pValue, valueSize, count);
	}

	uint32_t GetValueSize(uint32_t valueSize, uint32_t index = 0) {
		return m_valueSizes[index];
	}

	void SetValueSize(uint32_t valueSize, uint32_t index = 0);

	uint32_t GetFixedSize() {
		return m_fixedValueSize;
	}

	void SetFixedSize(uint32_t fixedSize);

	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);
protected:
	uint32_t		m_fixedValueSize;
	uint32_t		m_defaultValueSize;
	MP4Integer32Array	m_valueSizes;
	MP4BytesArray		m_values;
};

class MP4TableProperty : public MP4Property {
public:
	MP4TableProperty(const char *name, MP4IntegerProperty* pCountProperty);

	~MP4TableProperty();

	MP4PropertyType GetType() {
		return TableProperty;
	}

	void SetParentAtom(MP4Atom* pParentAtom) {
		m_pParentAtom = pParentAtom;
		for (uint32_t i = 0; i < m_pProperties.Size(); i++) {
			m_pProperties[i]->SetParentAtom(pParentAtom);
		}
	}

	void AddProperty(MP4Property* pProperty);

	MP4Property* GetProperty(uint32_t index) {
		return m_pProperties[index];
	}

	virtual uint32_t GetCount() {
	  return m_pCountProperty->GetValue();
	}
	virtual void SetCount(uint32_t count) {
	  m_pCountProperty->SetValue(count);
	}

	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);
	bool FindProperty(const char* name,
		MP4Property** ppProperty, uint32_t* pIndex = NULL);

protected:
	virtual void ReadEntry(MP4File* pFile, uint32_t index);
	virtual void WriteEntry(MP4File* pFile, uint32_t index);

	bool FindContainedProperty(const char* name, MP4Property** ppProperty, uint32_t* pIndex);

protected:
	MP4IntegerProperty*	m_pCountProperty;
	MP4PropertyArray	m_pProperties;
};

class MP4DescriptorProperty : public MP4Property {
public:
	MP4DescriptorProperty(const char *name = NULL, 
	  uint8_t tagsStart = 0, uint8_t tagsEnd = 0,
	  bool mandatory = false, bool onlyOne = false);

	~MP4DescriptorProperty();

	MP4PropertyType GetType() {
		return DescriptorProperty;
	}

	void SetParentAtom(MP4Atom* pParentAtom);

	void SetSizeLimit(uint64_t sizeLimit) {
		m_sizeLimit = sizeLimit;
	}

	uint32_t GetCount() {
		return m_pDescriptors.Size();
	}
	void SetCount(uint32_t count) {
		m_pDescriptors.Resize(count);
	}

	void SetTags(uint8_t tagsStart, uint8_t tagsEnd = 0) {
		m_tagsStart = tagsStart;
		m_tagsEnd = tagsEnd ? tagsEnd : tagsStart;
	}

	MP4Descriptor* AddDescriptor(uint8_t tag);

	void AppendDescriptor(MP4Descriptor* pDescriptor) {
		m_pDescriptors.Add(pDescriptor);
	}

	void DeleteDescriptor(uint32_t index);

	void Generate();
	void Read(MP4File* pFile, uint32_t index = 0);
	void Write(MP4File* pFile, uint32_t index = 0);
	void Dump(FILE* pFile, uint8_t indent, bool dumpImplicits, uint32_t index = 0);
	bool FindProperty(const char* name,
		MP4Property** ppProperty, uint32_t* pIndex = NULL);

protected:
	virtual MP4Descriptor* CreateDescriptor(uint8_t tag);

	bool FindContainedProperty(const char* name,
		MP4Property** ppProperty, uint32_t* pIndex);

protected:
	uint8_t			m_tagsStart;
	uint8_t			m_tagsEnd;
	uint64_t			m_sizeLimit;
	bool				m_mandatory;
	bool				m_onlyOne;
	MP4DescriptorArray	m_pDescriptors;
};

class MP4QosQualifierProperty : public MP4DescriptorProperty {
public:
	MP4QosQualifierProperty(const char *name = NULL, 
	  uint8_t tagsStart = 0, uint8_t tagsEnd = 0,
	  bool mandatory = false, bool onlyOne = false) :
	MP4DescriptorProperty(name, tagsStart, tagsEnd, mandatory, onlyOne) { }

protected:
	MP4Descriptor* CreateDescriptor(uint8_t tag);
};

