#include <ecs/SerializedEntitiesAndComponents.h>

#include <ecs/ComponentID.h>
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
	// Serialize the components.
	const uint32_t numComponentTypes = serialization.m_components.Size();
	outputFn(&numComponentTypes, sizeof(numComponentTypes));

	for (const auto& entry : serialization.m_components)
	{
		const char* const componentTypeName = Util::ReverseHash(entry.first.GetTypeHash());
		const uint32_t componentTypeNameLength = static_cast<uint32_t>(strlen(componentTypeName));
		outputFn(&componentTypeNameLength, sizeof(componentTypeNameLength));
		outputFn(componentTypeName, componentTypeNameLength);

		const auto& components = entry.second;

		const uint32_t numComponents = components.m_views.Size();
		outputFn(&numComponents, sizeof(numComponents));

		const size_t numComponentViewBytes = numComponents * sizeof(SerializedByteView);
		outputFn(components.m_views.begin(), numComponentViewBytes);

		const uint32_t numComponentBytes = components.m_bytes.Size();
		outputFn(&numComponentBytes, sizeof(numComponentBytes));
		outputFn(components.m_bytes.begin(), numComponentBytes);
	}

	// Serialize the entities.
	const uint32_t numEntities = serialization.m_entities.m_views.Size();
	outputFn(&numEntities, sizeof(numEntities));

	const uint32_t numEntityViewBytes = numEntities * sizeof(SerializedByteView);
	outputFn(serialization.m_entities.m_views.begin(), numEntityViewBytes);

	const uint32_t numEntityBytes = serialization.m_entities.m_bytes.Size();
	outputFn(&numEntityBytes, sizeof(numEntityBytes));
	outputFn(serialization.m_entities.m_bytes.begin(), numEntityBytes);
}

bool ECS::TryReadSerializedEntitiesAndComponentsFrom(
	const Collection::ArrayView<const uint8_t> fileBytes,
	SerializedEntitiesAndComponents& outSerialization)
{
	// TODO(network) TryReadSerializedEntitiesAndComponentsFrom
	// Read the component views.
	/*const uint8_t* iter = fileBytes.begin();
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

	iter += numBytes;*/

	return true;
}

namespace Internal_SerializedEntitiesAndComponents
{
static const uint32_t k_fullComponentsSectionMarker = 'CMPD';
static const uint32_t k_deltaComponentsSectionMarker = 'CMPF';
static const uint32_t k_entitiesSectionMarker = 'ENTI';

static const uint8_t k_elementRemovedMarker = 0xB5;
static const uint8_t k_elementDeltaMarker = 0xDE;
static const uint8_t k_elementAddedMarker = 0xAD;

template <typename T>
Collection::ArrayView<const uint8_t> CreateByteView(const Collection::Vector<T>& v)
{
	return { reinterpret_cast<const uint8_t*>(v.begin()), sizeof(T) * v.Size() };
}

template <typename HeaderType, typename IDType>
IDType GetInvalidIDForHeaderType();

template <>
uint64_t GetInvalidIDForHeaderType<ECS::FullSerializedComponentHeader, uint64_t>()
{
	return ECS::ComponentID::sk_invalidUniqueID;
}

template <>
uint32_t GetInvalidIDForHeaderType<ECS::FullSerializedEntityHeader, uint32_t>()
{
	return ECS::EntityID::sk_invalidValue;
}

template <typename HeaderType, typename IDType>
IDType GetIDFromHeader(const HeaderType& header);

template <>
uint64_t GetIDFromHeader<ECS::FullSerializedComponentHeader, uint64_t>(const ECS::FullSerializedComponentHeader& header)
{
	return header.m_uniqueID;
}

template <>
uint32_t GetIDFromHeader<ECS::FullSerializedEntityHeader, uint32_t>(const ECS::FullSerializedEntityHeader& header)
{
	return header.m_entityID.GetUniqueID();
}

template <typename HeaderType, typename IDType>
void DeltaCompressSortedLists(
	const ECS::SerializedBytesWithViews& lastSeenBytesWithViews,
	const ECS::SerializedBytesWithViews& newestBytesWithViews,
	Collection::Vector<uint8_t>& outBytes)
{
	const IDType invalidElementID = GetInvalidIDForHeaderType<HeaderType, IDType>();

	// Transmit the views.
	// TODO(network) evaluate whether or not this is a good way to transmit views.
	Network::DeltaCompression::Compress(
		CreateByteView(lastSeenBytesWithViews.m_views),
		CreateByteView(newestBytesWithViews.m_views),
		outBytes);

	// Scan over the sorted lists to transmit the data.
	const auto& lastSeenBytes = lastSeenBytesWithViews.m_bytes;
	const auto& newestBytes = newestBytesWithViews.m_bytes;

	const ECS::SerializedByteView* const lastSeenViewsEnd = lastSeenBytesWithViews.m_views.end();
	const ECS::SerializedByteView* lastSeenViewsIter = lastSeenBytesWithViews.m_views.begin();
	for (const auto& newestView : newestBytesWithViews.m_views)
	{
		const uint8_t* const rawNewestElement = &newestBytes[newestView.m_beginIndex];
		const auto& newestElementHeader = *reinterpret_cast<const HeaderType*>(rawNewestElement);
		const IDType newestElementID = GetIDFromHeader<HeaderType, IDType>(newestElementHeader);
		const size_t newestElementSize = newestView.m_endIndex - newestView.m_beginIndex;

		// Scan forward over the last seen list while the last seen ID is less than the newest ID.
		// Each element skipped is an element that was removed.
		IDType lastSeenElementID = invalidElementID;
		for (; lastSeenViewsIter < lastSeenViewsEnd; ++lastSeenViewsIter)
		{
			const HeaderType& elementHeader = *reinterpret_cast<const HeaderType*>(
				&lastSeenBytes[lastSeenViewsIter->m_beginIndex]);
			const IDType elementID = GetIDFromHeader<HeaderType, IDType>(elementHeader);

			if (elementID >= newestElementID)
			{
				lastSeenElementID = elementID;
				break;
			}

			// Transmit a removal marker.
			Mem::LittleEndian::Serialize(k_elementRemovedMarker, outBytes);
			Mem::LittleEndian::Serialize(elementID, outBytes);
		}

		// If the IDs match, we can perform delta compression. If they don't, this is a new element.
		if (lastSeenElementID == newestElementID)
		{
			// Transmit a delta marker.
			Mem::LittleEndian::Serialize(k_elementDeltaMarker, outBytes);
			Mem::LittleEndian::Serialize(newestElementID, outBytes);

			// Transmit the delta compressed element.
			const size_t lastSeenElementSize = lastSeenViewsIter->m_endIndex - lastSeenViewsIter->m_beginIndex;
			const uint8_t* const rawLastSeenElement = &lastSeenBytes[lastSeenViewsIter->m_beginIndex];

			Network::DeltaCompression::Compress(
				{ rawLastSeenElement, lastSeenElementSize },
				{ rawNewestElement, newestElementSize },
				outBytes);
		}
		else
		{
			// Transmit an element added marker.
			Mem::LittleEndian::Serialize(k_elementAddedMarker, outBytes);
			Mem::LittleEndian::Serialize(newestElementID, outBytes);

			// Transmit the entire added element.
			outBytes.AddAll({ rawNewestElement, newestElementSize });
		}
	}
}

bool TryDeltaDecompressSortedLists(const ECS::SerializedBytesWithViews& lastSeenBytesWithViews,
	const uint32_t numNewestElements,
	const uint8_t*& deltaCompressedIter,
	const uint8_t* const deltaCompressedEnd,
	ECS::SerializedBytesWithViews& outNewestBytesWithViews)
{
	// Delta-decompress the views.
	const size_t numNewestViewsBytes = numNewestElements * sizeof(ECS::SerializedByteView);

	outNewestBytesWithViews.m_views.Resize(numNewestElements);
	Collection::ArrayView<uint8_t> newestViewsView{
		reinterpret_cast<uint8_t*>(outNewestBytesWithViews.m_views.begin()), numNewestViewsBytes };

	if (!Network::DeltaCompression::TryDecompress(
		CreateByteView(lastSeenBytesWithViews.m_views),
		deltaCompressedIter,
		deltaCompressedEnd,
		newestViewsView))
	{
		return false;
	}

	// Ensure that the correct number of bytes were decompressed for the views.
	if (newestViewsView.Size() != numNewestViewsBytes)
	{
		return false;
	}

	// TODO(network) delta-decompress the elements
	return true;
}
}

void ECS::DeltaCompressSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const SerializedEntitiesAndComponents& newestFrame,
	Collection::Vector<uint8_t>& outBytes)
{
	using namespace Internal_SerializedEntitiesAndComponents;

	// Transmit the components.
	Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestFrame.m_components.Size()), outBytes);
	for (const auto& entry : newestFrame.m_components)
	{
		// Transmit a marker indicating whether or not delta compression was used for this set of components.
		const auto* const lastSeenEntryIter = lastSeenFrame.m_components.Find(entry.first);
		if (lastSeenEntryIter == lastSeenFrame.m_components.end())
		{
			Mem::LittleEndian::Serialize(k_fullComponentsSectionMarker, outBytes);
		}
		else
		{
			Mem::LittleEndian::Serialize(k_deltaComponentsSectionMarker, outBytes);
		}

		// Transmit the component type.
		const char* const componentTypeName = Util::ReverseHash(entry.first.GetTypeHash());
		Mem::LittleEndian::Serialize(componentTypeName, outBytes);

		// Transmit the number of components of this type.
		const Collection::Vector<ECS::SerializedByteView>& newestComponentViews = entry.second.m_views;
		Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestComponentViews.Size()), outBytes);

		// Fully transmit the component views and bytes when there are no last seen components.
		if (lastSeenEntryIter == lastSeenFrame.m_components.end())
		{
			const Collection::Vector<uint8_t>& newestComponentBytes = entry.second.m_bytes;
			Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestComponentBytes.Size()), outBytes);
			outBytes.AddAll(CreateByteView(newestComponentViews));
			outBytes.AddAll(newestComponentBytes.GetConstView());
			continue;
		}

		// Delta compress the component views and bytes when there are last seen components.
		DeltaCompressSortedLists<ECS::FullSerializedComponentHeader, uint64_t>(
			lastSeenEntryIter->second, entry.second, outBytes);

		// Scan over the sorted component lists to transmit the component data.
		/*const ECS::SerializedByteView* const lastSeenViewsEnd = lastSeenComponentViews.end();
		const ECS::SerializedByteView* lastSeenViewsIter = lastSeenComponentViews.begin();
		for (const auto& newestView : newestComponentViews)
		{
			const uint8_t* const rawNewestComponent = &newestComponentBytes[newestView.m_beginIndex];
			const auto& newestComponentHeader =
				*reinterpret_cast<const ECS::FullSerializedComponentHeader*>(rawNewestComponent);
			const size_t newestComponentSize = newestView.m_endIndex - newestView.m_beginIndex;

			// Scan forward over the last seen list while the last seen ID is less than the newest ID.
			// Each component skipped is a component that was removed.
			uint64_t lastSeenComponentID = ECS::ComponentID::sk_invalidUniqueID;
			for (; lastSeenViewsIter < lastSeenViewsEnd; ++lastSeenViewsIter)
			{
				const auto* componentHeader = reinterpret_cast<const ECS::FullSerializedComponentHeader*>(
					&lastSeenComponentBytes[lastSeenViewsIter->m_beginIndex]);
				if (componentHeader->m_uniqueID >= newestComponentHeader.m_uniqueID)
				{
					lastSeenComponentID = componentHeader->m_uniqueID;
					break;
				}

				// Transmit a removal marker.
				Mem::LittleEndian::Serialize(k_componentRemovedMarker, outBytes);
				Mem::LittleEndian::Serialize(componentHeader->m_uniqueID, outBytes);
			}

			// If the IDs match, we can perform delta compression. If they don't, this is a new component.
			if (lastSeenComponentID == newestComponentHeader.m_uniqueID)
			{
				// Transmit a delta marker.
				Mem::LittleEndian::Serialize(k_componentDeltaMarker, outBytes);
				Mem::LittleEndian::Serialize(newestComponentHeader.m_uniqueID, outBytes);

				// Transmit the delta compressed component.
				const size_t lastSeenComponentSize = lastSeenViewsIter->m_endIndex - lastSeenViewsIter->m_beginIndex;
				const uint8_t* const rawLastSeenComponent = &lastSeenComponentBytes[lastSeenViewsIter->m_beginIndex];

				Network::DeltaCompression::Compress(
					{ rawLastSeenComponent, lastSeenComponentSize },
					{ rawNewestComponent, newestComponentSize },
					outBytes);
			}
			else
			{
				// Transmit a component added marker.
				Mem::LittleEndian::Serialize(k_componentAddedMarker, outBytes);
				Mem::LittleEndian::Serialize(newestComponentHeader.m_uniqueID, outBytes);

				// Transmit the entire added component.
				outBytes.AddAll({ rawNewestComponent, newestComponentSize });
			}
		}*/
	}

	// TODO(network) handle component types that were removed entirely since the last seen frame

	// Transmit the entities.
	Mem::LittleEndian::Serialize(k_entitiesSectionMarker, outBytes);
	Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestFrame.m_entities.m_views.Size()), outBytes);
	{
		// Transmit the entity views.
		// TODO(network) evaluate whether or not this is a good way to transmit the entity views.
		Network::DeltaCompression::Compress(
			CreateByteView(lastSeenFrame.m_entities.m_views),
			CreateByteView(newestFrame.m_entities.m_views),
			outBytes);

		// Scan over the sorted entity lists to transmit the entity data.
		DeltaCompressSortedLists<ECS::FullSerializedEntityHeader, uint32_t>(
			lastSeenFrame.m_entities, newestFrame.m_entities, outBytes);
	}
}

bool ECS::TryDeltaDecompressSerializedEntitiesAndComponentsFrom(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const Collection::ArrayView<const uint8_t> deltaCompressedBytes,
	SerializedEntitiesAndComponents& outDecompressedSerialization)
{
	using namespace Internal_SerializedEntitiesAndComponents;
	using namespace Mem::LittleEndian;

	const uint8_t* deltaCompressedIter = deltaCompressedBytes.begin();
	const uint8_t* const deltaCompressedEnd = deltaCompressedBytes.end();

	// Decompress the component views and bytes.
	const auto maybeNumComponentTypes = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	if (!maybeNumComponentTypes.second)
	{
		return false;
	}
	const uint32_t numComponentTypes = maybeNumComponentTypes.first;
	for (size_t i = 0; i < numComponentTypes; ++i)
	{
		// Read the section marker.
		const auto maybeSectionMarker = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
		if (!maybeSectionMarker.second)
		{
			return false;
		}
		if (maybeSectionMarker.first != k_fullComponentsSectionMarker
			&& maybeSectionMarker.first != k_deltaComponentsSectionMarker)
		{
			return false;
		}
		const uint32_t sectionMarker = maybeSectionMarker.first;

		// Read the component type name.
		char componentTypeBuffer[64];
		if (!DeserializeString(deltaCompressedIter, deltaCompressedEnd, componentTypeBuffer))
		{
			return false;
		}
		const ECS::ComponentType componentType{ Util::CalcHash(componentTypeBuffer) };
		const auto* const lastSeenEntryIter = lastSeenFrame.m_components.Find(componentType);
		auto& outComponents = outDecompressedSerialization.m_components[componentType];

		// Read the number of components of this type.
		const auto maybeNumComponents = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
		if (!maybeNumComponents.second)
		{
			return false;
		}
		const uint32_t numComponents = maybeNumComponents.first;

		// Decompress the components.
		if (sectionMarker == k_fullComponentsSectionMarker)
		{
			if (lastSeenEntryIter != lastSeenFrame.m_components.end())
			{
				return false;
			}

			const auto maybeNumComponentBytes = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
			if (!maybeNumComponentBytes.second)
			{
				return false;
			}
			const uint32_t numComponentBytes = maybeNumComponentBytes.first;

			const size_t numComponentViewsBytes = numComponents * sizeof(ECS::SerializedByteView);
			if ((deltaCompressedIter + numComponentBytes + numComponentViewsBytes) >= deltaCompressedEnd)
			{
				return false;
			}

			outComponents.m_views.Resize(numComponents);
			memcpy(outComponents.m_views.begin(), deltaCompressedIter, numComponentViewsBytes);
			deltaCompressedIter += numComponentViewsBytes;

			outComponents.m_bytes.Resize(numComponentBytes);
			memcpy(outComponents.m_bytes.begin(), deltaCompressedIter, numComponentBytes);
			deltaCompressedIter += numComponentBytes;
		}
		else
		{
			AMP_FATAL_ASSERT(sectionMarker == k_deltaComponentsSectionMarker, "");
			if (lastSeenEntryIter == lastSeenFrame.m_components.end())
			{
				return false;
			}

			if (!TryDeltaDecompressSortedLists(
				lastSeenEntryIter->second, numComponents, deltaCompressedIter, deltaCompressedEnd, outComponents))
			{
				return false;
			}
		}
	}

	// Decompress the entities.
	const auto maybeEntitiesSectionMarker = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	const auto maybeNumEntities = DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	if (maybeEntitiesSectionMarker.second == false
		|| maybeEntitiesSectionMarker.first != k_entitiesSectionMarker
		|| maybeNumEntities.second == false)
	{
		return false;
	}
	const uint32_t numEntities = maybeNumEntities.first;
	
	if (!TryDeltaDecompressSortedLists(
		lastSeenFrame.m_entities,
		numEntities,
		deltaCompressedIter,
		deltaCompressedEnd,
		outDecompressedSerialization.m_entities))
	{
		return false;
	}

	return true;
}
