#include <client/ClientNetworkWorld.h>

#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

Client::ClientNetworkWorld::ClientNetworkWorld(const char* hostName, const char* hostPort)
	: m_socket(Network::CreateConnectedSocket(hostName, hostPort))
{
	if (m_socket.IsValid())
	{
		// Get the client ID from the host.
		uint8_t inboundBuffer[4096];
		Collection::ArrayView<uint8_t> inboundBufferView{ inboundBuffer, sizeof(inboundBuffer) };

		size_t numBytesReceived = m_socket.Receive(inboundBufferView);
		while (m_socket.IsValid())
		{
			Host::MessageToClient messageFromHost;
			if (TryReceiveMessageFromHost({ inboundBuffer, numBytesReceived }, messageFromHost))
			{
				if (messageFromHost.Is<Host::NotifyOfHostConnected_MessageToClient>())
				{
					m_clientID = messageFromHost.Get<Host::NotifyOfHostConnected_MessageToClient>().m_clientID;
					break;
				}
				// It's possible for messages to arrive out of order, so keep any other messages we encounter.
				else if (!m_hostToClientMessages.TryPush(std::move(messageFromHost)))
				{
					// If we fill the entire inbound message queue without receiving our client ID, we almost certainly
					// aren't going to get one.
					break;
				}
			}
			numBytesReceived = m_socket.Receive(inboundBufferView);
		}

		// Start the network thread only if we receive the client ID because it runs until the client ID is invalid.
		if (m_clientID.IsValid())
		{
			m_networkThread = std::thread(&ClientNetworkWorld::NetworkThreadFunction, this);
		}
	}
}

bool Client::ClientNetworkWorld::IsRunning() const
{
	return m_networkThread.joinable();
}

void Client::ClientNetworkWorld::WaitForShutdown()
{
	m_networkThread.join();
}

void Client::ClientNetworkWorld::NetworkThreadFunction()
{
	// Run the thread so long as the client ID is valid.
	while (m_clientID.IsValid())
	{
		// Receive any pending data from the host.
		uint8_t inboundBuffer[UINT16_MAX];
		Collection::ArrayView<uint8_t> inboundBufferView{ inboundBuffer, sizeof(inboundBuffer) };

		size_t numBytesReceived = m_socket.Receive(inboundBufferView);
		while (m_socket.IsValid() && numBytesReceived != 0)
		{
			Host::MessageToClient messageFromHost;
			if (TryReceiveMessageFromHost({ inboundBuffer, numBytesReceived }, messageFromHost))
			{
				if (!m_hostToClientMessages.TryPush(std::move(messageFromHost)))
				{
					// TODO(network)
					break;
				}
			}
			numBytesReceived = m_socket.Receive(inboundBufferView);
		}

		// Transmit client messages to the host.
		Client::MessageToHost messageToHost;
		while (m_socket.IsValid() && m_clientToHostMessages.TryPop(messageToHost))
		{
			if (messageToHost.Is<Client::MessageToHost_Disconnect>())
			{
				m_clientID = ClientID();
			}

			TransmitMessageToHost(messageToHost);
		}

		// If the socket is no longer valid, we are no longer a client.
		if (!m_socket.IsValid())
		{
			m_hostToClientMessages.TryPush(
				Host::MessageToClient::Make<Host::NotifyOfHostDisconnected_MessageToClient>());
			break;
		}

		std::this_thread::yield();
	}
}

bool Client::ClientNetworkWorld::TryReceiveMessageFromHost(
	const Collection::ArrayView<const uint8_t>& bytes,
	Host::MessageToClient& outMessage) const
{
	AMP_LOG("Received [%zu] bytes from host.", bytes.Size());

	const uint8_t* bytesIter = bytes.begin();
	const uint8_t* const bytesEnd = bytes.end();

	const auto maybeSequenceNumber = Mem::LittleEndian::DeserializeUi64(bytesIter, bytesEnd);
	const auto maybeTag = Mem::LittleEndian::DeserializeUi16(bytesIter, bytesEnd);
	if (maybeSequenceNumber.second == false || maybeTag.second == false)
	{
		return false;
	}

	const uint64_t sequenceNumber = maybeSequenceNumber.first;
	const uint16_t tag = maybeTag.first;

	switch (tag)
	{
	case 0: // NotifyOfHostConnected_MessageToClient
	{
		const auto maybeClientID = Mem::LittleEndian::DeserializeUi16(bytesIter, bytesEnd);
		if (maybeClientID.second == false)
		{
			return false;
		}

		outMessage = Host::MessageToClient::Make<Host::NotifyOfHostConnected_MessageToClient>();
		Host::NotifyOfHostConnected_MessageToClient& payload = outMessage.Get<0>();
		payload.m_clientID = ClientID(maybeClientID.first);
		return true;
	}
	case 1: // NotifyOfHostDisconnected_MessageToClient
	{
		outMessage = Host::MessageToClient::Make<Host::NotifyOfHostDisconnected_MessageToClient>();
		return true;
	}
	case 2: // ECSUpdate_MessageToClient
	{
		const auto maybeNumBytes = Mem::LittleEndian::DeserializeUi32(bytesIter, bytesEnd);
		if (maybeNumBytes.second == false || (bytesIter + maybeNumBytes.first) > bytesEnd)
		{
			return false;
		}
		const uint32_t numBytes = maybeNumBytes.first;

		outMessage = Host::MessageToClient::Make<Host::ECSUpdate_MessageToClient>();
		Host::ECSUpdate_MessageToClient& payload = outMessage.Get<2>();
		payload.m_bytes.Resize(numBytes);
		memcpy(payload.m_bytes.begin(), bytesIter, numBytes);
		bytesIter += numBytes;
		return true;
	}
	default:
	{
		AMP_LOG_WARNING("Received message from host with unknown tag [%u].", tag);
		return false;
	}
	}
}

void Client::ClientNetworkWorld::TransmitMessageToHost(const Client::MessageToHost& message)
{
	Collection::Vector<uint8_t> transmissionBuffer(4096);

	Mem::LittleEndian::Serialize(message.m_clientID.GetN(), transmissionBuffer);

	const uint16_t tag = static_cast<uint16_t>(message.GetTag());
	Mem::LittleEndian::Serialize(tag, transmissionBuffer);

	message.Match(
		[](const MessageToHost_Connect&) {},
		[](const MessageToHost_Disconnect&) {},
		[&](const MessageToHost_FrameAcknowledgement& payload)
		{
			Mem::LittleEndian::Serialize(payload.m_frameIndex, transmissionBuffer);
		},
		[&](const MessageToHost_InputStates& payload)
		{
			Mem::LittleEndian::Serialize(static_cast<uint32_t>(payload.m_bytes.Size()), transmissionBuffer);
			transmissionBuffer.AddAll(payload.m_bytes.GetConstView());
		});

	AMP_LOG("Sending [%u] bytes to host.", transmissionBuffer.Size());
	m_socket.Send(transmissionBuffer.GetConstView());
}
