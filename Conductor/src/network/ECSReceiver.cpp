#include <network/ECSReceiver.h>

#include <mem/DeserializeLittleEndian.h>

namespace Network
{
const ECS::SerializedEntitiesAndComponents* ECSReceiver::TryReceiveFrameTransmission(
	const Collection::ArrayView<const uint8_t> transmissionBytes)
{
	const uint8_t* transmissionIter = transmissionBytes.begin();
	const uint8_t* const transmissionEnd = transmissionBytes.end();

	// The first 4 bytes of the transmission indicate the type of the frame.
	const auto maybeFrameTypeMarker = Mem::LittleEndian::DeserializeUi32(transmissionIter, transmissionEnd);
	if (!maybeFrameTypeMarker.second)
	{
		return nullptr;
	}
	const uint32_t frameTypeMarker = maybeFrameTypeMarker.first;

	uint64_t receivedFrameIndex = k_invalidFrameIndex;
	if (frameTypeMarker == k_fullFrameMarker)
	{
		receivedFrameIndex = TryReceiveFullFrameTransmission(transmissionIter, transmissionEnd);
	}
	else if (frameTypeMarker == k_deltaFrameMarker)
	{
		receivedFrameIndex = TryReceiveDeltaFrameTransmission(transmissionIter, transmissionEnd);
	}

	if (receivedFrameIndex == k_invalidFrameIndex)
	{
		return nullptr;
	}
	return (receivedFrameIndex == m_frameIndex) ? &m_frameHistory.Newest().m_frame : nullptr;
}

uint64_t ECSReceiver::TryReceiveFullFrameTransmission(const uint8_t* const frameBegin, const uint8_t* const frameEnd)
{
	const uint8_t* frameIter = frameBegin;

	// Read the new frame index.
	const auto maybeNewFrameIndex = Mem::LittleEndian::DeserializeUi64(frameIter, frameEnd);
	if (maybeNewFrameIndex.second == false)
	{
		return k_invalidFrameIndex;
	}
	const uint64_t newFrameIndex = maybeNewFrameIndex.first;

	// If the frame doesn't fit within the frame history, we don't need to receive it.
	const uint64_t oldestStoredFrameIndex = m_frameIndex - (m_frameHistory.Size() - 1);
	if (m_frameIndex != k_invalidFrameIndex && newFrameIndex < oldestStoredFrameIndex)
	{
		return k_invalidFrameIndex;
	}

	// Receive a full frame.
	if (frameIter >= frameEnd)
	{
		return k_invalidFrameIndex;
	}
	const size_t numFrameBytes = (frameEnd - frameIter);

	ECS::SerializedEntitiesAndComponents newFrame;
	if (!ECS::TryReadSerializedEntitiesAndComponentsFrom({ frameIter, numFrameBytes }, newFrame))
	{
		return k_invalidFrameIndex;
	}

	StoreFrame(newFrameIndex, std::move(newFrame));
	return newFrameIndex;
}

uint64_t ECSReceiver::TryReceiveDeltaFrameTransmission(const uint8_t* const frameBegin, const uint8_t* const frameEnd)
{
	const uint8_t* frameIter = frameBegin;

	// If there is no frame history, there is no way to receive a delta frame.
	if (m_frameIndex == k_invalidFrameIndex)
	{
		return k_invalidFrameIndex;
	}

	// Read the new frame index and the index of the frame this transmission was compressed against.
	const auto maybeNewFrameIndex = Mem::LittleEndian::DeserializeUi64(frameIter, frameEnd);
	const auto maybePreviousFrameIndex = Mem::LittleEndian::DeserializeUi64(frameIter, frameEnd);
	if (maybeNewFrameIndex.second == false || maybePreviousFrameIndex.second == false)
	{
		return k_invalidFrameIndex;
	}
	const uint64_t newFrameIndex = maybeNewFrameIndex.first;
	const uint64_t previousFrameIndex = maybePreviousFrameIndex.first;

	// If we have a valid frame for the previous frame index, we can attempt to delta-decompress the new one.
	const uint64_t oldestStoredFrameIndex = m_frameIndex - (m_frameHistory.Size() - 1);
	if (previousFrameIndex < oldestStoredFrameIndex || previousFrameIndex > m_frameIndex)
	{
		return k_invalidFrameIndex;
	}

	const size_t historyIndex = static_cast<size_t>(newFrameIndex - oldestStoredFrameIndex);
	const HistoryEntry& previousFrameHistoryEntry = m_frameHistory[historyIndex];
	if (!previousFrameHistoryEntry.m_isValid)
	{
		return k_invalidFrameIndex;
	}

	if (frameIter >= frameEnd)
	{
		return k_invalidFrameIndex;
	}
	const size_t numFrameBytes = (frameEnd - frameIter);

	ECS::SerializedEntitiesAndComponents decompressedFrame;
	ECS::RemovedEntitiesAndComponents removedEntitiesAndComponents;
	if (!ECS::TryDeltaDecompressSerializedEntitiesAndComponentsFrom(
		previousFrameHistoryEntry.m_frame,
		{ frameIter, numFrameBytes },
		decompressedFrame,
		removedEntitiesAndComponents))
	{
		return k_invalidFrameIndex;
	}

	StoreFrame(newFrameIndex, std::move(decompressedFrame));
	return newFrameIndex;
}

void ECSReceiver::StoreFrame(uint64_t newFrameIndex, ECS::SerializedEntitiesAndComponents&& newFrame)
{
	// If the history buffer is empty, add the frame to it.
	if (m_frameIndex == k_invalidFrameIndex)
	{
		m_frameIndex = newFrameIndex;

		HistoryEntry& historyEntry = m_frameHistory.AddRecycle();
		historyEntry.m_isValid = true;
		historyEntry.m_frame = std::move(newFrame);
		return;
	}

	// If the frame has a place within the current history buffer, place it there.
	const uint64_t oldestStoredFrameIndex = m_frameIndex - (m_frameHistory.Size() - 1);
	const size_t historyIndex = static_cast<size_t>(newFrameIndex - oldestStoredFrameIndex);
	if (historyIndex < m_frameHistory.Size())
	{
		HistoryEntry& historyEntry = m_frameHistory[historyIndex];
		historyEntry.m_isValid = true;
		historyEntry.m_frame = std::move(newFrame);
		return;
	}

	// If the frame is after the end of the current history buffer, advance the buffer to the new frame to add it.
	m_frameIndex = newFrameIndex;

	for (size_t i = 0, iEnd = (newFrameIndex - m_frameIndex); i < iEnd; ++i)
	{
		HistoryEntry& historyEntry = m_frameHistory.AddRecycle();
		historyEntry.m_isValid = false;
	}

	HistoryEntry& historyEntry = m_frameHistory.Newest();
	historyEntry.m_isValid = true;
	historyEntry.m_frame = std::move(newFrame);
}
}
