#pragma once

#include <cstdint>

namespace ECS
{
enum class EntityFlags : uint32_t
{
	None = 0,
	Marked = 1 << 0, // Helper flag for mark & sweep algorithms.
	Networked = 1 << 1, // Only networked entities are synchronized between client & server.
};

inline EntityFlags operator~(const EntityFlags& lhs)
{
	const uint32_t result = ~static_cast<uint32_t>(lhs);
	return static_cast<EntityFlags>(result);
}

inline EntityFlags operator|(const EntityFlags& lhs, const EntityFlags& rhs)
{
	const uint32_t result = static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs);
	return static_cast<EntityFlags>(result);
}
inline void operator|=(EntityFlags& lhs, const EntityFlags& rhs)
{
	lhs = lhs | rhs;
}

inline EntityFlags operator&(const EntityFlags& lhs, const EntityFlags& rhs)
{
	const uint32_t result = static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
	return static_cast<EntityFlags>(result);
}
inline void operator&=(EntityFlags& lhs, const EntityFlags& rhs)
{
	lhs = lhs & rhs;
}
}
