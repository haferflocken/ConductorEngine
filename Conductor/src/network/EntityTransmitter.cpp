#include <network/EntityTransmitter.h>

namespace Network
{
namespace Internal_EntityTransmitter
{
constexpr uint8_t k_unchangedSectionTypeID = 0x0F;
constexpr uint8_t k_changedSectionTypeID = 0xF0;
constexpr uint8_t k_trailingSectionTypeID = 0xAA;
constexpr uint8_t k_maxSectionSize = UINT8_MAX;

void DeltaCompress(
	const Collection::ArrayView<const uint8_t>& lastSeenBytes,
	const Collection::ArrayView<const uint8_t>& currentBytes,
	Collection::Vector<uint8_t>& outCompressedBytes)
{
	// We delta compress by searching for runs of identical bytes. To encode this, the compressed bytes consist of three
	// types of sections. A section begins with a two byte marker: a type and a size.
	
	// An unchanged section's size indicates how many bytes to read from the previous transmission.
	// An unchanged section ends immediately after its size; they are always 2 bytes.
	// A changed section's size indicates how many bytes to read from the current transmission.
	// A changed section ends size bytes after its marker.
	// A trailing section's size indicates how many bytes to read from the current transmission.
	// A trailing section ends size bytes after its marker.

	// Find runs of identical bytes and encode them as unchanged sections. Encode all other bytes in the overlapping
	// byte range as changed sections. Encode all bytes following the overlapping range in trailing sections.
	const size_t minByteCount =
		(lastSeenBytes.Size() < currentBytes.Size()) ? lastSeenBytes.Size() : currentBytes.Size();
	size_t i = 0;
	for (; i < minByteCount; /* CONTROLLED IN LOOP */)
	{
		const size_t rewindI = i;
		for (size_t j = 0; j < k_maxSectionSize && i < minByteCount; ++j, ++i)
		{
			if (lastSeenBytes[i] != currentBytes[i])
			{
				break;
			}
		}
		const size_t unchangedRunEnd = i;
		const size_t unchangedRunLength = unchangedRunEnd - rewindI;
		AMP_FATAL_ASSERT(unchangedRunLength <= k_maxSectionSize, "Sections can't exceed 256 bytes!");

		// An unchanged run is only worth encoding if its longer than a section marker.
		if (unchangedRunLength > 2)
		{
			outCompressedBytes.Add(k_unchangedSectionTypeID);
			outCompressedBytes.Add(static_cast<uint8_t>(unchangedRunLength));
			// i is already in the right place.
			continue;
		}

		// Rewind i to the start of this iteration.
		i = rewindI;

		// Step i forward until the 4 bytes after i are identical.
		bool foundNextUnchangedRun = false;
		for (size_t j = 0; j < k_maxSectionSize && i < (minByteCount - 3); ++j, ++i)
		{
			if (memcmp(lastSeenBytes.begin() + i, currentBytes.begin() + i, 3) == 0)
			{
				foundNextUnchangedRun = true;
				break;
			}
		}

		// A changed run is only added if the next unchanged run was found. If we reached the end of the overlapping
		// byte range, we just encode these bytes in a trailing section.
		if (!foundNextUnchangedRun)
		{
			// Rewind i before adding the trailing section.
			i = rewindI;
			break;
		}

		const size_t changedRunEnd = i;
		const size_t changedRunLength = changedRunEnd - rewindI;
		AMP_FATAL_ASSERT(changedRunLength <= k_maxSectionSize, "Sections can't exceed 256 bytes!");

		outCompressedBytes.Add(k_changedSectionTypeID);
		outCompressedBytes.Add(static_cast<uint8_t>(changedRunLength));
		outCompressedBytes.AddAll({ currentBytes.begin() + rewindI, changedRunLength });

		// i is already in the right place.
	}

	// Add trailing sections to ensure we don't drop any bytes from outside the overlapping range.
	while (i < currentBytes.Size())
	{
		const size_t remainingBytes = currentBytes.Size() - i;
		const size_t sectionSize = (remainingBytes < k_maxSectionSize) ? remainingBytes : k_maxSectionSize;

		outCompressedBytes.Add(k_trailingSectionTypeID);
		outCompressedBytes.Add(static_cast<uint8_t>(sectionSize));
		outCompressedBytes.AddAll({ currentBytes.begin() + i, sectionSize });

		i += sectionSize;
	}
}
}

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

void EntityTransmitter::AddSerializedFrame(ECS::SerializedEntitiesAndComponents&& newFrame)
{
	++m_frameIndex;
	m_frameHistory.Add(std::move(newFrame));
}

void EntityTransmitter::SetLastSeenFrameForClient(const Client::ClientID clientID, uint64_t frameIndex)
{
	auto* const entry = m_lastSeenFramePerClient.Find(clientID);
	entry->second = frameIndex;
}

// TODO(network) move this to SerializedEntitiesAndComponents.h/cpp
void EntityTransmitter::TransmitFrame(
	const Client::ClientID clientID,
	Collection::Vector<uint8_t>& outTransmission) const
{
	using namespace Internal_EntityTransmitter;

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

	// TODO(network) transmit the component views and entity views

	// Component views in a SerializedEntitiesAndComponents are sorted by component ID, so we can iterate over the
	// components in each type in each frame at the same time.
	for (const auto& entry : newestFrame.m_componentViews)
	{
		const Collection::Vector<ECS::SerializedByteView>& newestComponentViews = entry.second;

		const auto* const lastSeenEntryIter = lastSeenFrame.m_componentViews.Find(entry.first);
		if (lastSeenEntryIter == nullptr)
		{
			// TODO(network) handle when there are no last seen components of this type
			continue;
		}

		// Scan over the sorted component lists.
		const Collection::Vector<ECS::SerializedByteView>& lastSeenComponentViews = lastSeenEntryIter->second;

		const ECS::SerializedByteView* const lastSeenViewsEnd = lastSeenComponentViews.end();
		const ECS::SerializedByteView* lastSeenViewIter = lastSeenComponentViews.begin();
		for (const auto& newestView : newestComponentViews)
		{
			const auto& newestComponentHeader = *reinterpret_cast<const ECS::FullSerializedComponentHeader*>(
				&newestFrame.m_bytes[newestView.m_beginIndex]);

			// Scan forward over the last seen list while the last seen ID is less than the newest ID.
			// Each component skipped is a component that was removed.
			const auto* lastSeenComponentHeader = reinterpret_cast<const ECS::FullSerializedComponentHeader*>(
				&lastSeenFrame.m_bytes[lastSeenViewIter->m_beginIndex]);
			while (lastSeenComponentHeader->m_uniqueID < newestComponentHeader.m_uniqueID)
			{
				// TODO(network) transmit a removal marker

				++lastSeenViewIter;
				lastSeenComponentHeader = reinterpret_cast<const ECS::FullSerializedComponentHeader*>(
					&lastSeenFrame.m_bytes[lastSeenViewIter->m_beginIndex]);
			}

			// If the IDs match, we can perform delta compression. If they don't, this is a new component.
			if (lastSeenComponentHeader->m_uniqueID == newestComponentHeader.m_uniqueID)
			{
				// TODO(network) transmit a delta marker

				const size_t lastSeenComponentSize = lastSeenViewIter->m_endIndex - lastSeenViewIter->m_beginIndex;
				const size_t newestComponentSize = newestView.m_endIndex - newestView.m_beginIndex;

				const uint8_t* const lastSeenComponentBytes = reinterpret_cast<const uint8_t*>(lastSeenComponentHeader);
				const uint8_t* const newestComponentBytes = reinterpret_cast<const uint8_t*>(&newestComponentHeader);

				DeltaCompress(
					{ lastSeenComponentBytes, lastSeenComponentSize },
					{ newestComponentBytes, newestComponentSize },
					outTransmission);
			}
			else
			{
				// TODO(network) transmit a new component marker
			}
		}
	}

	// TODO(network) transmit the entity data
}

void EntityTransmitter::TransmitFullFrame(Collection::Vector<uint8_t>& outTransmission) const
{
	const ECS::SerializedEntitiesAndComponents& newestFrame = m_frameHistory.Newest();
	ECS::WriteSerializedEntitiesAndComponentsTo(newestFrame,
		[&](const void* data, size_t length)
		{
			outTransmission.AddAll({ reinterpret_cast<const uint8_t*>(data), length });
		});
}
}
