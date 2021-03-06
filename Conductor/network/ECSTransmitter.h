#pragma once

#include <client/ClientID.h>
#include <collection/RingBuffer.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <ecs/SerializedEntitiesAndComponents.h>

namespace Network
{
/**
 * Transmits the state of entities and components based on what state connected clients are aware of.
 */
class ECSTransmitter final
{
public:
	void NotifyOfClientConnected(const Client::ClientID clientID);
	void NotifyOfClientDisconnected(const Client::ClientID clientID);

	void AddSerializedFrame(ECS::SerializedEntitiesAndComponents&& newFrame);
	void NotifyOfFrameAcknowledgement(const Client::ClientID clientID, uint64_t frameIndex);

	void TransmitFrame(const Client::ClientID clientID, Collection::Vector<uint8_t>& outTransmission) const;
	void TransmitFullFrame(Collection::Vector<uint8_t>& outTransmission) const;

private:
	static constexpr size_t k_historySize = 16;

	uint64_t m_frameIndex{ 0 };

	Collection::VectorMap<Client::ClientID, uint64_t> m_lastSeenFramePerClient;

	Collection::RingBuffer<ECS::SerializedEntitiesAndComponents, k_historySize> m_frameHistory;
};
}
