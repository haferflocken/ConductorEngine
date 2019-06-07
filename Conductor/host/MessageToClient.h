#pragma once

#include <client/ClientID.h>
#include <collection/Variant.h>
#include <collection/Vector.h>

#include <cstdint>

namespace Host
{
struct NotifyOfHostConnected_MessageToClient
{
	Client::ClientID m_clientID;
};

struct NotifyOfHostDisconnected_MessageToClient {};

struct ECSUpdate_MessageToClient
{
	Collection::Vector<uint8_t> m_bytes;
};

using MessageToClient = Collection::Variant<
	NotifyOfHostConnected_MessageToClient,
	NotifyOfHostDisconnected_MessageToClient,
	ECSUpdate_MessageToClient>;
}
