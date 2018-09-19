#pragma once

#include <unit/UnitTempl.h>

#include <cstdint>

namespace Unit
{
struct ByteCount64;
struct WordCount32;

/**
 * A strongly typed byte quantity.
 */
struct ByteCount64 : public UnitTempl<ByteCount64, int64_t>
{
	ByteCount64() = default;

	explicit constexpr ByteCount64(BackingType n)
		: BaseType(n)
	{}

	operator WordCount32() const;

	// Define multiplying by size_t, as it is a common operation.
	constexpr ByteCount64 operator*(const size_t& rhs) const { return TrueType(m_n * static_cast<BackingType>(rhs)); }
	void operator*=(const size_t& rhs) { m_n *= static_cast<BackingType>(rhs); }
};

/**
 * A strongly typed word quantity.
 */
struct WordCount32 : public UnitTempl<WordCount32, int32_t>
{
	WordCount32() = default;

	explicit constexpr WordCount32(BackingType n)
		: BaseType(n)
	{}

	operator ByteCount64() const;

	// Define multiplying by size_t, as it is a common operation.
	constexpr WordCount32 operator*(const size_t& rhs) const { return TrueType(m_n * static_cast<BackingType>(rhs)); }
	void operator*=(const size_t& rhs) { m_n *= static_cast<BackingType>(rhs); }
};

inline ByteCount64::operator WordCount32() const { return WordCount32(static_cast<int32_t>(m_n / sizeof(size_t))); }
inline WordCount32::operator ByteCount64() const { return ByteCount64(static_cast<int64_t>(m_n) * sizeof(size_t)); }

template <typename T>
inline constexpr WordCount32 WordSizeOf()
{
	return WordCount32(static_cast<int32_t>((sizeof(T) + sizeof(size_t) - 1) / sizeof(size_t)));
}

template <typename T>
inline constexpr Unit::ByteCount64 WordSizeOfInBytes() { return ByteCount64(WordSizeOf<T>()); }

inline constexpr size_t AlignedSizeOf(size_t sizeInBytes, size_t alignInBytes)
{
	const size_t numAligned = (2 * sizeInBytes - 1) / alignInBytes;
	return numAligned * alignInBytes;
}

template <typename T>
inline constexpr size_t AlignedSizeOf()
{
	return AlignedSizeOf(sizeof(T), alignof(T));
}
}
