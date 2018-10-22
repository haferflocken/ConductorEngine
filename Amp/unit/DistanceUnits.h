#pragma once

#include <unit/UnitTempl.h>

#include <cstdint>
#include <type_traits>

namespace Unit
{
// Standard units.
struct Metres;
struct Millimetres;
struct Micrometres;

// 1024 multiples of micrometres.
struct KibiMicrometres;
struct MebiMicrometres;

struct Metres : public UnitTempl<Metres, int32_t>
{
	explicit constexpr Metres(BackingType n)
		: BaseType(n)
	{}

	explicit operator Millimetres() const;
	explicit operator Micrometres() const;
};

struct Millimetres : public UnitTempl<Millimetres, int32_t>
{
	explicit constexpr Millimetres(BackingType n)
		: BaseType(n)
	{}

	explicit operator Metres() const;
	explicit operator Micrometres() const;
};

struct Micrometres : public UnitTempl<Micrometres, int32_t>
{
	explicit constexpr Micrometres(BackingType n)
		: BaseType(n)
	{}

	explicit operator Metres() const;
	explicit operator Millimetres() const;
	explicit operator KibiMicrometres() const;
	explicit operator MebiMicrometres() const;
};

struct KibiMicrometres : public UnitTempl<KibiMicrometres, int32_t>
{
	explicit constexpr KibiMicrometres(BackingType n)
		: BaseType(n)
	{}

	explicit operator Micrometres() const;
	explicit operator MebiMicrometres() const;
};

struct MebiMicrometres : public UnitTempl<KibiMicrometres, int32_t>
{
	explicit constexpr MebiMicrometres(BackingType n)
		: BaseType(n)
	{}

	explicit operator Micrometres() const;
	explicit operator KibiMicrometres() const;
};

inline Metres::operator Millimetres() const { return Millimetres(m_n * 1000); }
inline Metres::operator Micrometres() const { return Micrometres(m_n * 1000 * 1000); }

inline Millimetres::operator Metres() const { return Metres(m_n / 1000); }
inline Millimetres::operator Micrometres() const { return Micrometres(m_n * 1000); }

inline Micrometres::operator Metres() const { return Metres(m_n / (1000 * 1000)); }
inline Micrometres::operator Millimetres() const { return Millimetres(m_n / 1000); }
inline Micrometres::operator KibiMicrometres() const { return KibiMicrometres(m_n >> 10); }
inline Micrometres::operator MebiMicrometres() const { return MebiMicrometres(m_n >> 20); }

inline KibiMicrometres::operator Micrometres() const { return Micrometres(m_n << 10); }
inline KibiMicrometres::operator MebiMicrometres() const { return MebiMicrometres(m_n >> 10); }

inline MebiMicrometres::operator Micrometres() const { return Micrometres(m_n << 20); }
inline MebiMicrometres::operator KibiMicrometres() const { return KibiMicrometres(m_n << 10); }
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<Unit::Metres> : std::true_type {};
template <> struct IsMemCopyAFullCopy<Unit::Millimetres> : std::true_type {};
template <> struct IsMemCopyAFullCopy<Unit::Micrometres> : std::true_type {};
template <> struct IsMemCopyAFullCopy<Unit::KibiMicrometres> : std::true_type {};
template <> struct IsMemCopyAFullCopy<Unit::MebiMicrometres> : std::true_type {};
}
