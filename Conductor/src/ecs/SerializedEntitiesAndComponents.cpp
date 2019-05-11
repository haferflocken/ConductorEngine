#include <ecs/SerializedEntitiesAndComponents.h>

#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

#include <ostream>

void ECS::WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization, std::ostream& fileOutput)
{
	WriteSerializedEntitiesAndComponentsTo(serialization,
		[&](const void* data, size_t length)
		{
			fileOutput.write(reinterpret_cast<const char*>(data), length);
		});
}

void ECS::WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization,
	const std::function<void(const void*, size_t)>& outputFn)
{
	// Serialize the component views.
	Collection::Vector<uint8_t> viewBytes;

	const uint32_t numComponentTypes = serialization.m_componentViews.Size();
	Mem::LittleEndian::Serialize(numComponentTypes, viewBytes);

	for (const auto& entry : serialization.m_componentViews)
	{
		const char* const componentTypeName = Util::ReverseHash(entry.first.GetTypeHash());
		Mem::LittleEndian::Serialize(componentTypeName, viewBytes);

		const auto& componentViews = entry.second;

		const uint32_t numComponentViews = componentViews.Size();
		Mem::LittleEndian::Serialize(numComponentViews, viewBytes);

		const uint32_t componentViewsIndex = viewBytes.Size();
		const uint32_t sizeOfComponentViews = numComponentViews * sizeof(SerializedByteView);
		viewBytes.Resize(viewBytes.Size() + sizeOfComponentViews);
		memcpy(&viewBytes[componentViewsIndex], &componentViews.Front(), sizeOfComponentViews);
	}

	// Serialize the entity views.
	const uint32_t numEntityViews = serialization.m_entityViews.Size();
	Mem::LittleEndian::Serialize(numEntityViews, viewBytes);

	const uint32_t entityViewsIndex = viewBytes.Size();
	const uint32_t sizeOfEntityViews = numEntityViews * sizeof(SerializedByteView);
	viewBytes.Resize(viewBytes.Size() + sizeOfEntityViews);
	memcpy(&viewBytes[entityViewsIndex], &serialization.m_entityViews.Front(), sizeOfEntityViews);

	// Write the serialized views to the output.
	outputFn(&viewBytes.Front(), viewBytes.Size());

	// Write the serialized entities and components to the output.
	const uint32_t numBytes = serialization.m_bytes.Size();
	outputFn(&numBytes, sizeof(numBytes));
	outputFn(&serialization.m_bytes.Front(), serialization.m_bytes.Size());
}

bool ECS::TryReadSerializedEntitiesAndComponentsFrom(
	const Collection::ArrayView<const uint8_t> fileBytes,
	SerializedEntitiesAndComponents& outSerialization)
{
	// Read the component views.
	const uint8_t* iter = fileBytes.begin();
	const uint8_t* const iterEnd = fileBytes.end();

	const auto maybeNumComponentTypes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumComponentTypes.second)
	{
		return false;
	}

	for (size_t i = 0; i < maybeNumComponentTypes.first; ++i)
	{
		char componentTypeNameBuffer[64];
		if (!Mem::LittleEndian::DeserializeString(iter, iterEnd, componentTypeNameBuffer))
		{
			return false;
		}
		const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeNameBuffer);

		const auto maybeNumComponentViews = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
		if (!maybeNumComponentViews.second)
		{
			return false;
		}
		const uint32_t numComponentViews = maybeNumComponentViews.first;
		const uint32_t sizeOfComponentViews = numComponentViews * sizeof(SerializedByteView);
		if ((iterEnd - iter) < sizeOfComponentViews)
		{
			return false;
		}

		auto& componentViews = outSerialization.m_componentViews[ComponentType(componentTypeHash)];

		const uint32_t componentViewsIndex = componentViews.Size();
		componentViews.Resize(componentViews.Size() + numComponentViews);
		memcpy(&componentViews[componentViewsIndex], iter, sizeOfComponentViews);

		iter += sizeOfComponentViews;
	}

	// Read the entity views.
	const auto maybeNumEntityViews = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumEntityViews.second)
	{
		return false;
	}
	const uint32_t numEntityViews = maybeNumEntityViews.first;
	const uint32_t sizeOfEntityViews = numEntityViews * sizeof(SerializedByteView);
	if ((iterEnd - iter) < sizeOfEntityViews)
	{
		return false;
	}

	const uint32_t entityViewsIndex = outSerialization.m_entityViews.Size();
	outSerialization.m_entityViews.Resize(outSerialization.m_entityViews.Size() + numEntityViews);
	memcpy(&outSerialization.m_entityViews[entityViewsIndex], iter, sizeOfEntityViews);

	iter += sizeOfEntityViews;

	// Read the serialized entities and components.
	const auto maybeNumBytes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumBytes.second)
	{
		return false;
	}
	const uint32_t numBytes = maybeNumBytes.first;
	if ((iterEnd - iter) < numBytes)
	{
		return false;
	}

	outSerialization.m_bytes.Resize(numBytes);
	memcpy(&outSerialization.m_bytes.Front(), iter, numBytes);

	iter += numBytes;

	return true;
}

namespace Internal_SerializedEntitiesAndComponents
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

void ECS::DeltaCompressSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const SerializedEntitiesAndComponents& newestFrame,
	Collection::Vector<uint8_t>& outBytes)
{
	using namespace Internal_SerializedEntitiesAndComponents;

	// Transmit the component views.
	// TODO(network) section marker
	{
	}

	// Transmit the entity views.
	// TODO(network) section marker
	{
		// TODO(network) evaluate whether or not this is a good way to transmit the entity views.
		const size_t numLastSeenEntityViewsBytes = lastSeenFrame.m_entityViews.Size() * sizeof(ECS::SerializedByteView);
		const size_t numNewestEntityViewsBytes = newestFrame.m_entityViews.Size() * sizeof(ECS::SerializedByteView);

		const auto lastSeenEntityViewsBytes = reinterpret_cast<const uint8_t*>(lastSeenFrame.m_entityViews.begin());
		const auto newestEntityViewsBytes = reinterpret_cast<const uint8_t*>(newestFrame.m_entityViews.begin());

		DeltaCompress(
			{ lastSeenEntityViewsBytes, numLastSeenEntityViewsBytes },
			{ newestEntityViewsBytes, numNewestEntityViewsBytes },
			outBytes);
	}

	// Component views in a SerializedEntitiesAndComponents are sorted by component ID, so we can iterate over the
	// components in each type in each frame at the same time.
	// TODO(network) section marker
	for (const auto& entry : newestFrame.m_componentViews)
	{
		const Collection::Vector<ECS::SerializedByteView>& newestComponentViews = entry.second;

		// TODO(network) transmit the component type

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
					outBytes);
			}
			else
			{
				// TODO(network) transmit a new component marker
			}
		}
	}

	// Transmit the entity data.
	// TODO(network) section marker
	{

	}
}

bool ECS::TryDeltaDecompressSerializedEntitiesAndComponentsFrom(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	Collection::ArrayView<const uint8_t> deltaCompressedBytes,
	SerializedEntitiesAndComponents& outDecompressedSerialization)
{
	// TODO(network) decompress
	return false;
}