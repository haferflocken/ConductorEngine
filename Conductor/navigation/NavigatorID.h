#pragma once

#include <util/UniqueID.h>

#include <cstdint>

namespace Navigation
{
/**
 * A unique identifier for a navigator.
 */
class NavigatorID : public Util::UniqueID<NavigatorID, uint32_t>
{
public:
	NavigatorID()
		: UniqueID()
	{}

	NavigatorID(const BackingType uniqueID)
		: UniqueID(uniqueID)
	{}
};
}

namespace Traits
{
template <typename T>
struct IsMemCopyAFullCopy;

template <> struct IsMemCopyAFullCopy<Navigation::NavigatorID> : std::true_type {};
}
