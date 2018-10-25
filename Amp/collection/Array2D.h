#pragma once

#include <dev/Dev.h>

#include <traits/IsMemCopyAFullCopy.h>

#include <algorithm>

namespace Collection
{
/**
 * Base class for rectangular 2D arrays. Constructors are protected so that it cannot be directly instantiated.
 * Instead, instantiate and use instances of Array2D.
 */
template <typename ElemType, size_t Width, size_t Height>
class Array2DBase
{
protected:
	ElemType m_values[Width * Height];

	Array2DBase()
		: m_values()
	{}

	static size_t FlatIndex(size_t x, size_t y) { return x + y * Width; }

public:
	using value_type = ElemType;
	using iterator = ElemType*;
	using const_iterator = const ElemType*;

	static constexpr size_t sk_width = Width;
	static constexpr size_t sk_height = Height;
	static constexpr size_t sk_size = Width * Height;

	ElemType& GetValue(size_t x, size_t y) { return m_values[FlatIndex(x, y)]; }
	const ElemType& GetValue(size_t x, size_t y) const { return m_values[FlatIndex(x, y)]; };

	void SetValue(size_t x, size_t y, const ElemType& value) { m_values[FlatIndex(x, y)] = value; }
	void SetValue(size_t x, size_t y, ElemType&& value) { m_values[FlatIndex(x, y)] = std::move(value); }

	iterator begin() { return std::begin(m_values); }
	const_iterator begin() const { return std::begin(m_values); }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return std::end(m_values); }
	const_iterator end() const { return std::end(m_values); }
	const_iterator cend() const { return end(); }
};

/**
 * A rectangular 2D array.
 */
template <typename ElemType, size_t Width, size_t Height, typename EnableMemCopyConstructor = void>
class Array2D : public Array2DBase<ElemType, Width, Height>
{
public:
	Array2D()
		: Array2DBase()
	{}

	Array2D(const Array2D<ElemType, Width, Height>& o)
		: Array2DBase()
	{
		std::copy_n(o.m_values, sk_size, m_values);
	}

	Array2D(Array2D<ElemType, Width, Height>&& o)
		: Array2DBase()
	{
		for (size_t i = 0; i < sk_size; ++i)
		{
			m_values[i] = std::move(o.m_values[i]);
		}
	}

	template <size_t OutWidth, size_t OutHeight>
	void CopyPortion(const size_t xMin, const size_t yMin, Array2D<ElemType, OutWidth, OutHeight>& destination)
	{
		static_assert(OutWidth <= Width && OutHeight <= Height,
			"CopyPortion requires a destination array with the same or smaller dimensions than the source.");
		AMP_FATAL_ASSERT(xMin + OutWidth <= Width && yMin + OutHeight <= Height,
			"[%zu, %zu] min results in out of bound copy.", xMin, yMin);

		for (size_t i = 0; i < OutWidth; ++i)
		{
			for (size_t j = 0; j < OutHeight; ++j)
			{
				destination.SetValue(i, j, GetValue(i + xMin, j + yMin));
			}
		}
	}
};

/**
 * A rectangular 2D array, specialized to use memcpy when copying the array.
 */
template <typename ElemType, size_t Width, size_t Height>
class Array2D<ElemType, Width, Height, std::enable_if_t<Traits::IsMemCopyAFullCopy<ElemType>::value>>
	: public Array2DBase<ElemType, Width, Height>
{
public:
	Array2D()
		: Array2DBase()
	{}

	Array2D(const Array2D<ElemType, Width, Height>& o)
		: Array2DBase()
	{
		memcpy(m_values, o.m_values, sizeof(ElemType) * sk_size);
	}

	template <size_t OutWidth, size_t OutHeight>
	void CopyPortion(const size_t xMin, const size_t yMin, Array2D<ElemType, OutWidth, OutHeight>& destination)
	{
		static_assert(OutWidth <= Width && OutHeight <= Height,
			"CopyPortion requires a destination array with the same or smaller dimensions than the source.");
		AMP_FATAL_ASSERT(xMin + OutWidth <= Width && yMin + OutHeight <= Height,
			"[%zu, %zu] min results in out of bound copy.", xMin, yMin);

		constexpr size_t numBytes = OutWidth * sizeof(ElemType);
		for (size_t j = 0; j < OutHeight; ++j)
		{
			const auto* rowStart = &m_values[FlatIndex(xMin, yMin + j)];
			memcpy(&destination.GetValue(0, j), rowStart, numBytes);
		}
	}
};
}

namespace Traits
{
/**
 * IsMemCopyAFullCopy specialization to indicate that an Array2D can be memcpy'd if its elements can.
 */
template<typename ElemType, size_t Width, size_t Height>
struct IsMemCopyAFullCopy<Collection::Array2D<ElemType, Width, Height>> : IsMemCopyAFullCopy<ElemType> {};
}
