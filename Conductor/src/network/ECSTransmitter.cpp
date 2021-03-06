#include <network/ECSTransmitter.h>

#include <mem/SerializeLittleEndian.h>
#include <network/ECSTransmission.h>

namespace Network
{
void ECSTransmitter::NotifyOfClientConnected(const Client::ClientID clientID)
{
	m_lastSeenFramePerClient[clientID] = k_invalidFrameIndex;
}

void ECSTransmitter::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	[[maybe_unused]] const bool removed = m_lastSeenFramePerClient.TryRemove(clientID);
	AMP_FATAL_ASSERT(removed,
		"Client [%u] disconnected without connecting first, or disconnected twice in a row!",
		static_cast<uint32_t>(clientID.GetN()));
}

void ECSTransmitter::AddSerializedFrame(ECS::SerializedEntitiesAndComponents&& newFrame)
{
	++m_frameIndex;
	m_frameHistory.Add(std::move(newFrame));
}

void ECSTransmitter::NotifyOfFrameAcknowledgement(const Client::ClientID clientID, uint64_t frameIndex)
{
	auto* const entry = m_lastSeenFramePerClient.Find(clientID);
	if (frameIndex > entry->second || entry->second == k_invalidFrameIndex)
	{
		entry->second = frameIndex;
	}
}

void ECSTransmitter::TransmitFrame(
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
	const uint64_t oldestStoredFrameIndex = m_frameIndex - (m_frameHistory.Size() - 1);
	if (lastSeenFrameIndex < oldestStoredFrameIndex)
	{
		TransmitFullFrame(outTransmission);
		return;
	}

	// If we have a last seen frame for this client, construct a delta transmission against the current frame.
	const size_t historyIndex = static_cast<size_t>(lastSeenFrameIndex - oldestStoredFrameIndex);
	const ECS::SerializedEntitiesAndComponents& lastSeenFrame = m_frameHistory[historyIndex];
	const ECS::SerializedEntitiesAndComponents& newestFrame = m_frameHistory.Newest();

	// Transmit the delta frame marker so that the receiver knows to decompress the data.
	Mem::LittleEndian::Serialize(k_deltaFrameMarker, outTransmission);
	
	// Transmit the current frame index and the index of the frame this is compressed against.
	Mem::LittleEndian::Serialize(m_frameIndex, outTransmission);
	Mem::LittleEndian::Serialize(lastSeenFrameIndex, outTransmission);

	// Create the delta transmission.
	ECS::DeltaCompressSerializedEntitiesAndComponentsTo(lastSeenFrame, newestFrame, outTransmission);
}

void ECSTransmitter::TransmitFullFrame(Collection::Vector<uint8_t>& outTransmission) const
{
	// Transmit the full frame marker so that the receiver knows to directly apply the data.
	Mem::LittleEndian::Serialize(k_fullFrameMarker, outTransmission);

	// Transmit the current frame index and create the full transmission.
	Mem::LittleEndian::Serialize(m_frameIndex, outTransmission);

	const ECS::SerializedEntitiesAndComponents& newestFrame = m_frameHistory.Newest();
	ECS::WriteSerializedEntitiesAndComponentsTo(newestFrame,
		[&](const void* data, size_t length)
		{
			outTransmission.AddAll({ reinterpret_cast<const uint8_t*>(data), length });
		});
}
}
