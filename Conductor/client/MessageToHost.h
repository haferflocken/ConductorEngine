#pragma once

#include <client/ClientID.h>
#include <collection/Variant.h>
#include <host/MessageToClient.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Client
{
struct MessageToHost_Connect final
{
	// m_hostToClientMessages is not actually transmitted, but it is used in host code to link HostWorld and HostNetworkWorld.
	Collection::LocklessQueue<Host::MessageToClient>* m_hostToClientMessages{ nullptr };
};

struct MessageToHost_Disconnect final {};

struct MessageToHost_FrameAcknowledgement final
{
	uint64_t m_frameIndex;
};

struct MessageToHost_InputStates final
{
	Collection::Vector<uint8_t> m_bytes;
};

struct MessageToHost final : public Collection::Variant<
	MessageToHost_Connect,
	MessageToHost_Disconnect,
	MessageToHost_FrameAcknowledgement,
	MessageToHost_InputStates>
{
	template <typename T, typename... Args>
	static MessageToHost Make(const Client::ClientID clientID, Args&&... args)
	{
		return MessageToHost(clientID, Variant::Make<T, Args...>(std::forward<Args>(args)...));
	}

	using Variant::Variant;

	MessageToHost() = default;
	
	MessageToHost(const Client::ClientID clientID, Variant&& v)
		: Variant(std::move(v))
		, m_clientID(clientID)
	{}

	Client::ClientID m_clientID{ 0 };
};
}
