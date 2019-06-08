#include <host/HostNetworkWorld.h>

#include <client/ConnectedHost.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

Host::HostNetworkWorld::HostNetworkWorld(const char* listenerPort)
	: m_listenerSocket(Network::CreateAndBindListenerSocket(listenerPort))
{
	if (m_listenerSocket.TryListen())
	{
		// Add the default local client for the server administrator.
		m_networkConnectedClients[k_localClientID] = Mem::MakeUnique<NetworkConnectedClient>();
		{
			Client::ConnectedHost localHost{ k_localClientID, m_clientToHostMessageQueue };
			localHost.Connect(&m_networkConnectedClients[k_localClientID]->m_hostToClientMessageQueue);
		}

		// Start the network thread after adding the default local client because the network thread terminates
		// when the default local client is disconnected.
		m_networkThread = std::thread(&HostNetworkWorld::NetworkThreadFunction, this);
	}
}

bool Host::HostNetworkWorld::IsRunning() const
{
	return m_networkThread.joinable();
}

void Host::HostNetworkWorld::WaitForShutdown()
{
	m_networkThread.join();
}

void Host::HostNetworkWorld::NotifyOfConsoleInput(std::string&& input)
{
	m_consoleMessageQueue.TryPush(std::move(input));
}

namespace Internal_HostNetworkWorld
{
void ProcessConsoleMessage(Client::ConnectedHost& localHost, const std::string& message)
{
	const char* const cMessage = message.c_str();
	if (strcmp(cMessage, "exit") == 0 || strcmp(cMessage, "quit") == 0)
	{
		localHost.Disconnect();
	}
	// TODO(network) kick clients
	// TODO(network) print client list
}
}

void Host::HostNetworkWorld::NetworkThreadFunction()
{
	// Run the thread so long as the default local client is not disconnected.
	while (m_networkConnectedClients.Find(k_localClientID) != m_networkConnectedClients.end())
	{
		// Process console messages.
		{
			// Create a Client::ConnectedHost for the local client to make processing simpler.
			// This is valid because Client::ConnectedHost just wraps this thread's client to host message queue.
			Client::ConnectedHost localHost{ k_localClientID, m_clientToHostMessageQueue };

			std::string message;
			while (m_consoleMessageQueue.TryPop(message))
			{
				Internal_HostNetworkWorld::ProcessConsoleMessage(localHost, message);
			}
		}

		// Accept client connection requests.
		constexpr size_t k_maxNewClients = 8;
		Network::Socket newClientSockets[k_maxNewClients];
		const size_t numNewClients = m_listenerSocket.AcceptPendingConnections(newClientSockets, k_maxNewClients);
		
		for (size_t i = 0; i < numNewClients; ++i)
		{
			const Client::ClientID clientID{
				static_cast<uint16_t>(k_localClientID.GetN() + m_networkConnectedClients.Size() + 1) };

			AMP_LOG("New client connection accepted. ID: %u", clientID.GetN());

			Mem::UniquePtr<NetworkConnectedClient>& networkConnectedClient = m_networkConnectedClients[clientID];
			networkConnectedClient = Mem::MakeUnique<NetworkConnectedClient>();
			networkConnectedClient->m_clientSocket = std::move(newClientSockets[i]);

			auto& message = Host::MessageToClient::Make<NotifyOfHostConnected_MessageToClient>();
			auto& payload = message.Get<NotifyOfHostConnected_MessageToClient>();
			payload.m_clientID = clientID;
			networkConnectedClient->m_hostToClientMessageQueue.TryPush(std::move(message));
		}

		// Process the network connected clients.
		Collection::Vector<Client::ClientID> disconnectedClientIDs;
		for (auto& entry : m_networkConnectedClients)
		{
			const Client::ClientID clientID = entry.first;
			Mem::UniquePtr<NetworkConnectedClient>& networkConnectedClient = entry.second;

			// Receive any pending data from the client.
			if (clientID != k_localClientID)
			{
				uint8_t inboundBuffer[4096];
				Collection::ArrayView<uint8_t> inboundBufferView{ inboundBuffer, sizeof(inboundBuffer) };
				
				size_t numBytesReceived = networkConnectedClient->m_clientSocket.Receive(inboundBufferView);
				while (networkConnectedClient->m_clientSocket.IsValid() && numBytesReceived != 0)
				{
					Client::MessageToHost messageFromClient;
					if (TryReceiveMessageFromClient(
						{ inboundBuffer, numBytesReceived },
						clientID,
						*networkConnectedClient,
						messageFromClient))
					{
						// TODO(network) should there be separate queues for each client?
						if (!m_clientToHostMessageQueue.TryPush(std::move(messageFromClient)))
						{
							// TODO(network)
							break;
						}
					}
					numBytesReceived = networkConnectedClient->m_clientSocket.Receive(inboundBufferView);
				}

				// If the socket is no longer valid, this client is no longer valid.
				if (!networkConnectedClient->m_clientSocket.IsValid())
				{
					disconnectedClientIDs.Add(clientID);
					continue;
				}
			}

			// TODO(network) Process client to host messages, including disconnection messages.
			
			// TODO(network) Check if the client disconnected without notice.

			// Transmit host messages to each client.
			Host::MessageToClient messageToClient;
			while (networkConnectedClient->m_hostToClientMessageQueue.TryPop(messageToClient))
			{
				if (messageToClient.Is<Host::NotifyOfHostDisconnected_MessageToClient>())
				{
					disconnectedClientIDs.Add(clientID);
				}

				// Messages to the local client are exchanged via shared thread-safe queue and don't use a socket.
				if (clientID != k_localClientID)
				{
					TransmitMessageToClient(messageToClient, *networkConnectedClient);
				}
			}
		}

		// Remove any clients that are no longer connected.
		for (const auto& clientID : disconnectedClientIDs)
		{
			m_networkConnectedClients.TryRemove(clientID);
		}

		std::this_thread::yield();
	}
}

bool Host::HostNetworkWorld::TryReceiveMessageFromClient(
	const Collection::ArrayView<const uint8_t>& bytes,
	const Client::ClientID expectedClientID,
	NetworkConnectedClient& networkConnectedClient,
	Client::MessageToHost& outMessage) const
{
	AMP_LOG("Received [%zu] bytes from client.", bytes.Size());

	const uint8_t* bytesIter = bytes.begin();
	const uint8_t* const bytesEnd = bytes.end();

	const auto maybeClientID = Mem::LittleEndian::DeserializeUi16(bytesIter, bytesEnd);
	const auto maybeTag = Mem::LittleEndian::DeserializeUi16(bytesIter, bytesEnd);
	if (maybeClientID.second == false || maybeTag.second == false)
	{
		return false;
	}
	const Client::ClientID clientID{ maybeClientID.first };
	const uint16_t tag = maybeTag.first;

	if (clientID != expectedClientID)
	{
		AMP_LOG_WARNING("Received a message with client ID [%u] on socket for client [%u]",
			clientID.GetN(), expectedClientID.GetN());
		return false;
	}

	switch (tag)
	{
	case 0: // MessageToHost_Connect
	{
		// Construct a MessageToHost_Connect with a pointer to the message queue for the client.
		// This is part of the abstraction that makes HostNetworkWorld optional.
		outMessage = Client::MessageToHost::Make<Client::MessageToHost_Connect>(clientID);
		Client::MessageToHost_Connect& payload = outMessage.Get<0>();
		payload.m_hostToClientMessages = &networkConnectedClient.m_hostToClientMessageQueue;
		return true;
	}
	case 1: // MessageToHost_Disconnect
	{
		return true;
	}
	case 2: // MessageToHost_FrameAcknowledgement
	{
		const auto maybeFrameIndex = Mem::LittleEndian::DeserializeUi64(bytesIter, bytesEnd);
		if (maybeFrameIndex.second == false)
		{
			return false;
		}

		outMessage = Client::MessageToHost::Make<Client::MessageToHost_FrameAcknowledgement>(clientID);
		Client::MessageToHost_FrameAcknowledgement& payload = outMessage.Get<2>();
		payload.m_frameIndex = maybeFrameIndex.first;
		return true;
	}
	case 3: // MessageToHost_InputStates
	{
		const auto maybeNumBytes = Mem::LittleEndian::DeserializeUi32(bytesIter, bytesEnd);
		if (maybeNumBytes.second == false || (bytesIter + maybeNumBytes.first) > bytesEnd)
		{
			return false;
		}
		const uint32_t numBytes = maybeNumBytes.first;

		outMessage = Client::MessageToHost::Make<Client::MessageToHost_InputStates>(clientID);
		Client::MessageToHost_InputStates& payload = outMessage.Get<3>();
		payload.m_bytes.Resize(numBytes);
		memcpy(payload.m_bytes.begin(), bytesIter, numBytes);
		bytesIter += numBytes;
		return true;
	}
	default:
	{
		AMP_LOG_WARNING("Received message from client [%u] with unknown tag [%u].", clientID.GetN(), tag);
		return false;
	}
	}
}

void Host::HostNetworkWorld::TransmitMessageToClient(
	const Host::MessageToClient& message, NetworkConnectedClient& networkConnectedClient)
{
	Collection::Vector<uint8_t> transmissionBuffer(4096);

	const uint64_t sequenceNumber = networkConnectedClient.m_nextSequenceNumber++;
	Mem::LittleEndian::Serialize(sequenceNumber, transmissionBuffer);

	const uint16_t tag = static_cast<uint16_t>(message.GetTag());
	Mem::LittleEndian::Serialize(tag, transmissionBuffer);

	message.Match(
		[&](const NotifyOfHostConnected_MessageToClient& payload)
		{
			Mem::LittleEndian::Serialize(payload.m_clientID.GetN(), transmissionBuffer);
		},
		[](const NotifyOfHostDisconnected_MessageToClient&) {},
		[&](const ECSUpdate_MessageToClient& payload)
		{
			Mem::LittleEndian::Serialize(static_cast<uint32_t>(payload.m_bytes.Size()), transmissionBuffer);
			transmissionBuffer.AddAll(payload.m_bytes.GetConstView());
		});

	AMP_LOG("Sending [%u] bytes to client.", transmissionBuffer.Size());
	networkConnectedClient.m_clientSocket.Send(transmissionBuffer.GetConstView());
}
