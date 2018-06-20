#pragma once

#include <limits>

namespace Util
{
template <typename TT, typename BT>
class UniqueID
{
public:
	using TrueType = TT;
	using BackingType = BT;
	using SelfType = UniqueID<TT, BT>;

	static constexpr BackingType sk_invalidValue = std::numeric_limits<BackingType>::max();

	UniqueID()
		: m_value(sk_invalidValue)
	{}

	UniqueID(const SelfType& o) = default;
	UniqueID& operator=(const SelfType& rhs) = default;

	explicit UniqueID(const BackingType uniqueID)
		: m_value(uniqueID)
	{}

	BackingType GetUniqueID() const { return m_value; }

	bool operator==(const SelfType& rhs) const { return m_value == rhs.m_value; }
	bool operator!=(const SelfType& rhs) const { return m_value != rhs.m_value; }
	bool operator<(const SelfType& rhs) const { return m_value < rhs.m_value; }
	bool operator<=(const SelfType& rhs) const { return m_value <= rhs.m_value; }
	bool operator>(const SelfType& rhs) const { return m_value > rhs.m_value; }
	bool operator>=(const SelfType& rhs) const { return m_value >= rhs.m_value; }

private:
	BackingType m_value;
};
}
