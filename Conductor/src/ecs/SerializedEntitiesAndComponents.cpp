#include <ecs/SerializedEntitiesAndComponents.h>

#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>
#include <network/DeltaCompression.h>

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
static const uint32_t k_componentViewsSectionMarker = 'CMPV';
static const uint32_t k_entityViewsSectionMarker = 'ENTV';

static const uint32_t k_componentsSectionMarker = 'COMP';
static const uint32_t k_entitiesSectionMarker = 'ENTI';
}

void ECS::DeltaCompressSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const SerializedEntitiesAndComponents& newestFrame,
	Collection::Vector<uint8_t>& outBytes)
{
	using namespace Internal_SerializedEntitiesAndComponents;

	// TODO(network) Transmit the component views.
	Mem::LittleEndian::Serialize(k_componentViewsSectionMarker, outBytes);
	{
	}

	// Transmit the entity views.
	Mem::LittleEndian::Serialize(k_entityViewsSectionMarker, outBytes);
	Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestFrame.m_entityViews.Size()), outBytes);
	{
		// TODO(network) evaluate whether or not this is a good way to transmit the entity views.
		const size_t numLastSeenEntityViewsBytes = lastSeenFrame.m_entityViews.Size() * sizeof(ECS::SerializedByteView);
		const size_t numNewestEntityViewsBytes = newestFrame.m_entityViews.Size() * sizeof(ECS::SerializedByteView);

		const auto lastSeenEntityViewsBytes = reinterpret_cast<const uint8_t*>(lastSeenFrame.m_entityViews.begin());
		const auto newestEntityViewsBytes = reinterpret_cast<const uint8_t*>(newestFrame.m_entityViews.begin());

		Network::DeltaCompression::Compress(
			{ lastSeenEntityViewsBytes, numLastSeenEntityViewsBytes },
			{ newestEntityViewsBytes, numNewestEntityViewsBytes },
			outBytes);
	}

	// Component views in a SerializedEntitiesAndComponents are sorted by component ID, so we can iterate over the
	// components in each type in each frame at the same time.
	Mem::LittleEndian::Serialize(k_componentsSectionMarker, outBytes);
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

				Network::DeltaCompression::Compress(
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

	// TODO(network) Transmit the entity data.
	Mem::LittleEndian::Serialize(k_entitiesSectionMarker, outBytes);
	{

	}
}

bool ECS::TryDeltaDecompressSerializedEntitiesAndComponentsFrom(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const Collection::ArrayView<const uint8_t> deltaCompressedBytes,
	SerializedEntitiesAndComponents& outDecompressedSerialization)
{
	// TODO(network) decompress
	return false;
}
