#pragma once

#include <util/UniqueID.h>

#include <cstdint>

namespace Behave
{
/**
 * Actors are identified by ID.
 */
class ActorID : public Util::UniqueID<ActorID, uint32_t>
{
public:
	ActorID()
		: UniqueID()
	{}

	explicit ActorID(const BackingType uniqueID)
		: UniqueID(uniqueID)
	{}
};
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<Behave::ActorID> : std::true_type {};
}
