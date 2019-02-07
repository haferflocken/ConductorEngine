#pragma once

#include <dev/Dev.h>
#include <unit/CountUnits.h>

#include <cstring>

namespace Collection
{
/*
 * A dynamically allocated buffer whose elements may be of any type.
 * Elements of the buffer are not constructed with the buffer; they must be constructed explicitly.
 * Elements are always contiguous; there are never gaps in the buffer.
 */
class PolyBuffer
{
public:
	PolyBuffer() = default;

	PolyBuffer(const Unit::ByteCount64 capacityInBytes)
		: m_start(new uint8_t[capacityInBytes.GetN()])
		, m_maximum(m_start + capacityInBytes.GetN())
		, m_top(m_start)
	{}

	PolyBuffer(const PolyBuffer&) = delete;
	PolyBuffer& operator=(const PolyBuffer&) = delete;

	PolyBuffer(PolyBuffer&& o) noexcept
		: m_start(o.m_start)
		, m_maximum(o.m_maximum)
		, m_top(o.m_top)
	{
		o.m_start = nullptr;
		o.m_maximum = nullptr;
		o.m_top = nullptr;
	}

	void operator=(PolyBuffer&& rhs) noexcept
	{
		deleteOwned();
		m_start = rhs.m_start;
		m_maximum = rhs.m_maximum;
		m_top = rhs.m_top;
		rhs.m_start = nullptr;
		rhs.m_maximum = nullptr;
		rhs.m_top = nullptr;
	}

	~PolyBuffer()
	{
		deleteOwned();
	}

	Unit::ByteCount64 capacity() const { return Unit::ByteCount64(m_maximum - m_start); }
	Unit::ByteCount64 size() const { return Unit::ByteCount64(m_top - m_start); }

	bool empty() const { return (m_top == m_start); }

	template <typename T>
	bool hasRoomFor() const;

	template <typename T>
	T* peek();

	template <typename T>
	const T* peek() const;

	template <typename T, typename... Args>
	T& emplace(Args&&... args);

	void pop();

	void memcpyMoveAll(PolyBuffer& destination);

private:
	struct ElementMetadata
	{
		Unit::WordCount32 objectSize;
		void(*objectDestructor)(const void*);	
	};
	static constexpr Unit::WordCount32 sk_metadataSize{ Unit::WordSizeOf<ElementMetadata>() };

	const ElementMetadata* peekMetadata() const
	{
		return reinterpret_cast<const ElementMetadata*>(m_top - Unit::ByteCount64(sk_metadataSize).GetN());
	}

	void deleteOwned()
	{
		if (m_start != nullptr)
		{
			while (!empty())
			{
				pop();
			}
			delete[] m_start;
		}
	}

	// The beginning of the buffer.
	uint8_t* m_start{ nullptr };

	// The maximum value that the top can have.
	uint8_t* m_maximum{ nullptr };

	// The very top of the buffer.
	uint8_t* m_top{ nullptr };
};

template <typename T>
inline bool PolyBuffer::hasRoomFor() const
{
	return ((m_top + Unit::WordSizeOfInBytes<T>().GetN() + Unit::ByteCount64(sk_metadataSize).GetN()) <= m_maximum);
}

template <typename T>
inline T* PolyBuffer::peek()
{
	AMP_FATAL_ASSERT(!empty(), "Cannot peek an empty buffer!");

	const ElementMetadata& metadata = *peekMetadata();
	return reinterpret_cast<T*>(m_top - Unit::ByteCount64(metadata.objectSize + sk_metadataSize).GetN());
}

template <typename T>
inline const T* PolyBuffer::peek() const
{
	AMP_FATAL_ASSERT(!empty(), "Cannot peek an empty buffer!");

	const ElementMetadata& metadata = *peekMetadata();
	return reinterpret_cast<const T*>(m_top - Unit::ByteCount64(metadata.objectSize + sk_metadataSize).GetN());
}

template <typename T, typename... Args>
inline T& PolyBuffer::emplace(Args&&... args)
{
	AMP_FATAL_ASSERT(hasRoomFor<T>(), "Buffer does not have room for an object of that type.");

	// Interpret the memory at the top as a T* and construct a T there.
	T* const obj = reinterpret_cast<T*>(m_top);
	new (obj) T(std::forward<Args>(args)...);

	// Write the metadata directly after it.
	constexpr Unit::WordCount32 sizeOfT = Unit::WordSizeOf<T>();
	ElementMetadata& metadata = *reinterpret_cast<ElementMetadata*>(m_top + Unit::ByteCount64(sizeOfT).GetN());
	metadata.objectSize = sizeOfT;
	metadata.objectDestructor = [](const void* e) { static_cast<const T*>(e)->~T(); };

	// Shift the top past the object and its metadata.
	m_top += Unit::ByteCount64(sizeOfT + sk_metadataSize).GetN();

	return *obj;
}

inline void PolyBuffer::pop()
{
	AMP_FATAL_ASSERT(!empty(), "Cannot pop an empty buffer!");

	// Call the object's destructor.
	const ElementMetadata& metadata = *peekMetadata();
	void* const obj = static_cast<void*>(m_top - Unit::ByteCount64(metadata.objectSize + sk_metadataSize).GetN());
	metadata.objectDestructor(obj);

	// Move the top offset back so the object and its metadata will be overwritten.
	m_top -= Unit::ByteCount64(metadata.objectSize + sk_metadataSize).GetN();
}

inline void PolyBuffer::memcpyMoveAll(PolyBuffer& destination)
{
	void* const rawDestination = static_cast<void*>(destination.m_start);
	void* const rawSource = static_cast<void*>(m_start);

	memcpy(rawDestination, rawSource, Unit::ByteCount64(size()).GetN());
	destination.m_top = destination.m_start + size().GetN();
	
	m_top = m_start;
}
}
