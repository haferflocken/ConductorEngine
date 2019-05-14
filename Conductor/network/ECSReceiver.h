#pragma once

#include <collection/ArrayView.h>
#include <collection/RingBuffer.h>
#include <ecs/SerializedEntitiesAndComponents.h>
#include <network/ECSTransmission.h>

namespace Network
{
/**
 * Receives and processes transmitted entity component system state.
 */
class ECSReceiver final
{
public:
	// Receives a frame transmission. Returns a pointer to the frame if the frame is newer than any known frame.
	// Returns nullptr otherwise.
	const ECS::SerializedEntitiesAndComponents* TryReceiveFrameTransmission(
		const Collection::ArrayView<const uint8_t> transmissionBytes);

private:
	struct HistoryEntry
	{
		bool m_isValid{ false };
		ECS::SerializedEntitiesAndComponents m_frame{};
	};

	static constexpr size_t k_historySize = 16;

	// These return the frame index of the frame received, or k_invalidFrameIndex if the frame wasn't received.
	uint64_t TryReceiveFullFrameTransmission(const uint8_t* const frameBegin, const uint8_t* const frameEnd);
	uint64_t TryReceiveDeltaFrameTransmission(const uint8_t* const frameBegin, const uint8_t* const frameEnd);

	void StoreFrame(uint64_t newFrameIndex, ECS::SerializedEntitiesAndComponents&& newFrame);

private:
	uint64_t m_frameIndex{ k_invalidFrameIndex };
	Collection::RingBuffer<HistoryEntry, k_historySize> m_frameHistory;
};
}
