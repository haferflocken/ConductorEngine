#pragma once

namespace Unit
{

template <typename TT, typename BT>
class UnitTempl
{
public:
	typedef TT TrueType;
	typedef BT BackingType;

	UnitTempl()
		: m_n()
	{}

	explicit constexpr UnitTempl(BackingType n)
		: m_n(n)
	{}

	constexpr BackingType GetN() const { return m_n; }

	constexpr TrueType operator+(const TrueType& rhs) const { return TrueType(m_n + rhs.m_n); }
	constexpr TrueType operator-(const TrueType& rhs) const { return TrueType(m_n - rhs.m_n); }
	constexpr TrueType operator*(const BackingType& rhs) const { return TrueType(m_n * rhs); }
	constexpr TrueType operator/(const BackingType& rhs) const { return TrueType(m_n / rhs); }
	constexpr TrueType operator%(const BackingType& rhs) const { return TrueType(m_n % rhs); }

	TrueType operator++() { return TrueType(++m_n); }
	TrueType operator++(int) { return TrueType(m_n++); }

	void operator+=(const TrueType& rhs) { m_n += rhs.m_n; }
	void operator-=(const TrueType& rhs) { m_n -= rhs.m_n; }
	void operator*=(const BackingType& rhs) { m_n *= rhs; }
	void operator/=(const BackingType& rhs) { m_n /= rhs; }
	void operator%=(const BackingType& rhs) { m_n %= rhs; }

	constexpr bool operator==(const TrueType& rhs) const { return m_n == rhs.m_n; }
	constexpr bool operator!=(const TrueType& rhs) const { return m_n != rhs.m_n; }
	constexpr bool operator<(const TrueType& rhs) const { return m_n < rhs.m_n; }
	constexpr bool operator<=(const TrueType& rhs) const { return m_n <= rhs.m_n; }
	constexpr bool operator>(const TrueType& rhs) const { return m_n > rhs.m_n; }
	constexpr bool operator>=(const TrueType& rhs) const { return m_n >= rhs.m_n; }

protected:
	typedef UnitTempl<TT, BT> BaseType;

	BackingType m_n;
};

}
