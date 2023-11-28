#pragma once

#include "mp4util.h"

typedef uint32_t MP4ArrayIndex;

class MP4Array {
public:
	MP4Array() {
		m_numElements = 0;
		m_maxNumElements = 0;
	}

	inline bool ValidIndex(MP4ArrayIndex index) {
		if (m_numElements == 0 || index > m_numElements - 1) {
			return false;
		}
		return true;
	}

	inline MP4ArrayIndex Size(void) {
		return m_numElements;
	}

	inline MP4ArrayIndex MaxSize(void) {
		return m_maxNumElements;
	}

protected:
	MP4ArrayIndex	m_numElements;
	MP4ArrayIndex	m_maxNumElements;
};

// macro to generate subclasses
// we use this as an alternative to templates
// due to the excessive compile time price of extensive template usage

template <class type>
	class MP4TArray : public MP4Array 
	{ 
	public: 
		MP4TArray() { 
			m_elements = NULL; 
		} 
		
		~MP4TArray() { 
			MP4Free(m_elements); 
		} 
		
		inline void Add(type newElement) { 
			Insert(newElement, m_numElements); 
		} 
		
		void Insert(type newElement, MP4ArrayIndex newIndex) { 
			if (newIndex > m_numElements) { 
				throw new MP4Error(ERANGE, "MP4Array::Insert"); 
			}
			if (m_numElements == m_maxNumElements) {
				m_maxNumElements = MAX(m_maxNumElements, 1) * 2;
				m_elements = (type*)MP4ReallocArray(m_elements,
					m_maxNumElements, sizeof(type));
			}
			memmove(&m_elements[newIndex + 1], &m_elements[newIndex],
				(m_numElements - newIndex) * sizeof(type));
			m_elements[newIndex] = newElement;
			m_numElements++;
		} 
		
		void Delete(MP4ArrayIndex index) {
			if (!ValidIndex(index)) { 
				throw new MP4Error(ERANGE, "MP4Array::Delete"); 
			} 
			m_numElements--; 
			if (index < m_numElements) { 
			  memmove(&m_elements[index], &m_elements[index + 1], 
			   	  (m_numElements - index) * sizeof(type)); 
			} 
		} 

		void Resize(MP4ArrayIndex newSize) { 
			m_numElements = newSize; 
			m_maxNumElements = newSize; 
			m_elements = (type*)MP4ReallocArray(m_elements, 
				m_maxNumElements, sizeof(type)); 
		} 
		
		type& operator[](MP4ArrayIndex index) { 
			if (!ValidIndex(index)) { 
				throw new MP4Error(ERANGE, "index %u of %u", "MP4Array::[]", index, m_numElements); 
			} 
			return m_elements[index]; 
		} 
		
	protected: 
		type*	m_elements; 
	};

#define MP4ARRAY_DECL(name, type) typedef MP4TArray<type> name##Array;

MP4ARRAY_DECL(MP4Integer8, uint8_t)
MP4ARRAY_DECL(MP4Integer16, uint16_t)
MP4ARRAY_DECL(MP4Integer32, uint32_t)
MP4ARRAY_DECL(MP4Integer64, uint64_t)
MP4ARRAY_DECL(MP4Float32, float)
MP4ARRAY_DECL(MP4Float64, double)
MP4ARRAY_DECL(MP4String, char*)
MP4ARRAY_DECL(MP4Bytes, uint8_t*)

