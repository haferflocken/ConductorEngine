#pragma once

#include <collection/Variant.h>
#include <collection/Vector.h>
#include <cstdint>

namespace Host
{
struct NotifyOfHostDisconnected_MessageToClient {};

struct ECSUpdate_MessageToClient
{
	Collection::Vector<uint8_t> m_bytes;
};

using MessageToClient = Collection::Variant<
	NotifyOfHostDisconnected_MessageToClient,
	ECSUpdate_MessageToClient>;
}
