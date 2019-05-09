#include <network/EntityTransmitter.h>

namespace Network
{
void EntityTransmitter::NotifyOfClientConnected(const Client::ClientID clientID)
{
	m_lastSeenFramePerClient[clientID] = k_invalidFrameIndex;
}

void EntityTransmitter::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	[[maybe_unused]] const bool removed = m_lastSeenFramePerClient.TryRemove(clientID);
	AMP_FATAL_ASSERT(removed,
		"Client [%u] disconnected without connecting first, or disconnected twice in a row!",
		static_cast<uint32_t>(clientID.GetN()));
}

void EntityTransmitter::AddSerializedFrame(ECS::SerializedEntitiesAndComponents&& serializedFrame)
{
	++m_frameIndex;
	m_frameHistory.Add(std::move(serializedFrame));
}

void EntityTransmitter::SetLastSeenFrameForClient(const Client::ClientID clientID, uint64_t frameIndex)
{
	auto* const entry = m_lastSeenFramePerClient.Find(clientID);
	entry->second = frameIndex;
}

void EntityTransmitter::TransmitFrame(
	const Client::ClientID clientID,
	Collection::Vector<uint8_t>& outTransmission) const
{
	// Fall back to a full transmission if there is no history or the client hasn't seen a frame yet.
	const uint64_t lastSeenFrameIndex = m_lastSeenFramePerClient.Find(clientID)->second;
	if (m_frameHistory.Size() == 0 || lastSeenFrameIndex == k_invalidFrameIndex)
	{
		TransmitFullFrame(outTransmission);
		return;
	}

	// Fall back to a full transmission if there's not enough history to create a delta frame.
	const uint64_t oldestStoredFrameIndex = m_frameIndex - m_frameHistory.Size() - 1;
	if (lastSeenFrameIndex < oldestStoredFrameIndex)
	{
		TransmitFullFrame(outTransmission);
		return;
	}

	// If we have a last seen frame for this client, construct a delta transmission against the current frame.
	const size_t historyIndex = static_cast<size_t>(lastSeenFrameIndex - oldestStoredFrameIndex);
	const ECS::SerializedEntitiesAndComponents& lastSeenFrame = m_frameHistory[historyIndex];
	const ECS::SerializedEntitiesAndComponents& newestFrame = m_frameHistory.Newest();

	// TODO(network) calculate a delta
}

void EntityTransmitter::TransmitFullFrame(Collection::Vector<uint8_t>& outTransmission) const
{
	const ECS::SerializedEntitiesAndComponents& frame = m_frameHistory.Newest();
	// TODO(network) serialize the full frame
}
}
