#pragma once

#include <host/MessageToClient.h>

#include <client/ClientID.h>
#include <client/MessageToHost.h>
#include <collection/LocklessQueue.h>
#include <collection/VectorMap.h>
#include <mem/UniquePtr.h>

#include <string>
#include <thread>

namespace Host
{
	class HostNetworkWorld
	{
	public:
		static constexpr size_t k_consoleMessageCapacity = 8;
		static constexpr size_t k_inboundMessageCapacity = 512;
		static constexpr size_t k_outboundMessageCapacityPerClient = 128;
		static constexpr Client::ClientID k_localClientID{ 1 };

		HostNetworkWorld();

		bool IsRunning() const;
		void WaitForShutdown();

		void NotifyOfConsoleInput(std::string&& input);

		Collection::LocklessQueue<Client::MessageToHost>& GetClientToHostMessageQueue() { return m_clientToHostMessageQueue; }

	private:
		// Network connect clients are stored in unique pointers so that their address does not change
		// even as the container holding them changes its ordering. This is necessary because Host::ConnectedClient
		// holds a reference to a NetworkConnectedClient's message queue.
		struct NetworkConnectedClient
		{
			Collection::LocklessQueue<Host::MessageToClient> m_hostToClientMessageQueue{ k_outboundMessageCapacityPerClient };
		};

		void NetworkThreadFunction();

		Collection::LocklessQueue<std::string> m_consoleMessageQueue{ k_consoleMessageCapacity };
		Collection::LocklessQueue<Client::MessageToHost> m_clientToHostMessageQueue{ k_inboundMessageCapacity };
		Collection::VectorMap<Client::ClientID, Mem::UniquePtr<NetworkConnectedClient>> m_networkConnectedClients{};

		std::thread m_networkThread{};
	};
}
