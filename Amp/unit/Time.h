#pragma once

#include <unit/UnitTempl.h>

#include <cstdint>
#include <type_traits>

namespace Unit::Time
{
struct Millisecond;
struct Second;

struct Millisecond : public UnitTempl<Millisecond, uint64_t>
{
	explicit constexpr Millisecond(BackingType n)
		: BaseType(n)
	{}

	operator Second() const;
};

struct Second : public UnitTempl<Second, uint64_t>
{
	explicit constexpr Second(BackingType n)
		: BaseType(n)
	{}

	operator Millisecond() const;
};

inline Millisecond::operator Second() const { return Second(m_n / 1000); }
inline Second::operator Millisecond() const { return Millisecond(m_n * 1000); }
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<Unit::Time::Millisecond> : std::true_type {};
template <> struct IsMemCopyAFullCopy<Unit::Time::Second> : std::true_type {};
}
