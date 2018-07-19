#pragma once

#include <util/UniqueID.h>

#include <cstdint>

namespace ECS
{
/**
 * Entities are identified by ID.
 */
class EntityID : public Util::UniqueID<EntityID, uint32_t>
{
public:
	EntityID()
		: UniqueID()
	{}

	explicit EntityID(const BackingType uniqueID)
		: UniqueID(uniqueID)
	{}
};
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<ECS::EntityID> : std::true_type {};
}
