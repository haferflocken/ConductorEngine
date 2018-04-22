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

	static constexpr BackingType sk_invalidUniqueID = std::numeric_limits<BackingType>::max();

	UniqueID()
		: m_uniqueID(sk_invalidUniqueID)
	{}

	UniqueID(const SelfType& o) = default;
	UniqueID& operator=(const SelfType& rhs) = default;

	explicit UniqueID(const BackingType uniqueID)
		: m_uniqueID(uniqueID)
	{}

	BackingType GetUniqueID() const { return m_uniqueID; }

	bool operator==(const SelfType& rhs) const { return m_uniqueID == rhs.m_uniqueID; }
	bool operator!=(const SelfType& rhs) const { return !(*this == rhs); }
	bool operator<(const SelfType& rhs) const { return m_uniqueID < rhs.m_uniqueID; }

private:
	BackingType m_uniqueID;
};
}
