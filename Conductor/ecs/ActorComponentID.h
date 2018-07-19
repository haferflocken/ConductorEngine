#pragma once

#include <util/StringHash.h>

namespace ECS
{
/**
 * Actor components are identified by ID.
 */
class ActorComponentID
{
public:
	static constexpr size_t sk_invalidUniqueID = std::numeric_limits<size_t>::max();

	ActorComponentID()
		: m_type()
		, m_id(sk_invalidUniqueID)
	{}

	ActorComponentID(const ActorComponentID&) = default;
	ActorComponentID& operator=(const ActorComponentID&) = default;

	ActorComponentID(const Util::StringHash type, const size_t uniqueID)
		: m_type(type)
		, m_id(uniqueID)
	{}

	Util::StringHash GetType() const { return m_type; }
	size_t GetUniqueID() const { return m_id; }

	bool operator==(const ActorComponentID& rhs) const { return m_type == rhs.m_type && m_id == rhs.m_id; }
	bool operator!=(const ActorComponentID& rhs) const { return !(*this == rhs); }
	bool operator<(const ActorComponentID& rhs) const
	{
		return m_type < rhs.m_type || (m_type == rhs.m_type && m_id < rhs.m_id);
	}
	
private:
	Util::StringHash m_type;
	size_t m_id;
};
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<ECS::ActorComponentID> : std::true_type {};
}
