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
	// Read the components.
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

		const auto maybeNumComponents = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
		if (!maybeNumComponents.second)
		{
			return false;
		}
		const uint32_t numComponents = maybeNumComponents.first;
		const uint32_t sizeOfComponentViews = numComponents * sizeof(SerializedByteView);
		if ((iter + sizeOfComponentViews) > iterEnd)
		{
			return false;
		}

		auto& components = outSerialization.m_components[ComponentType(componentTypeHash)];

		const uint32_t componentViewsIndex = components.m_views.Size();
		components.m_views.Resize(componentViewsIndex + numComponents);
		memcpy(&components.m_views[componentViewsIndex], iter, sizeOfComponentViews);
		iter += sizeOfComponentViews;

		const auto maybeNumComponentBytes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
		if (!maybeNumComponentBytes.second)
		{
			return false;
		}
		
		const uint32_t numComponentBytes = maybeNumComponentBytes.first;
		if ((iter + numComponentBytes) > iterEnd)
		{
			return false;
		}

		const uint32_t componentBytesIndex = components.m_bytes.Size();
		components.m_bytes.Resize(componentBytesIndex + numComponentBytes);
		memcpy(&components.m_bytes[componentBytesIndex], iter, numComponentBytes);
		iter += numComponentBytes;
	}

	// Read the entities.
	const auto maybeNumEntities = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumEntities.second)
	{
		return false;
	}
	const uint32_t numEntities = maybeNumEntities.first;
	const uint32_t sizeOfEntityViews = numEntities * sizeof(SerializedByteView);
	if ((iter + sizeOfEntityViews) > iterEnd)
	{
		return false;
	}

	const uint32_t entityViewsIndex = outSerialization.m_entities.m_views.Size();
	outSerialization.m_entities.m_views.Resize(entityViewsIndex + numEntities);
	memcpy(&outSerialization.m_entities.m_views[entityViewsIndex], iter, sizeOfEntityViews);
	iter += sizeOfEntityViews;

	const auto maybeNumEntityBytes = Mem::LittleEndian::DeserializeUi32(iter, iterEnd);
	if (!maybeNumEntityBytes.second)
	{
		return false;
	}
	const uint32_t numEntityBytes = maybeNumEntityBytes.first;
	if ((iter + numEntityBytes) > iterEnd)
	{
		return false;
	}

	const uint32_t entityBytesIndex = outSerialization.m_entities.m_bytes.Size();
	outSerialization.m_entities.m_bytes.Resize(entityBytesIndex + numEntityBytes);
	memcpy(&outSerialization.m_entities.m_bytes[entityBytesIndex], iter, numEntityBytes);
	iter += numEntityBytes;

	return true;
}

namespace Internal_SerializedEntitiesAndComponents
{
static const uint32_t k_fullComponentsSectionMarker = 'CMPD';
static const uint32_t k_deltaComponentsSectionMarker = 'CMPF';
static const uint32_t k_entitiesSectionMarker = 'ENTI';
static const uint32_t k_elementsRemovedSectionMarker = 'RMOV';

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

	// Scan over the sorted lists to transmit the data and determine which elements were removed.
	const auto& lastSeenBytes = lastSeenBytesWithViews.m_bytes;
	const auto& newestBytes = newestBytesWithViews.m_bytes;

	const ECS::SerializedByteView* const lastSeenViewsEnd = lastSeenBytesWithViews.m_views.end();
	const ECS::SerializedByteView* lastSeenViewsIter = lastSeenBytesWithViews.m_views.begin();

	Collection::Vector<IDType> removedElementIDs;
	for (const auto& newestView : newestBytesWithViews.m_views)
	{
		const uint8_t* const rawNewestElement = &newestBytes[newestView.m_beginIndex];
		const auto& newestElementHeader = *reinterpret_cast<const HeaderType*>(rawNewestElement);
		const IDType newestElementID = GetIDFromHeader<HeaderType, IDType>(newestElementHeader);
		const uint32_t newestElementSize = newestView.m_endIndex - newestView.m_beginIndex;

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

			removedElementIDs.Add(elementID);
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

	// Transmit the list of elements that were removed.
	Mem::LittleEndian::Serialize(k_elementsRemovedSectionMarker, outBytes);
	Mem::LittleEndian::Serialize(static_cast<uint32_t>(removedElementIDs.Size()), outBytes);
	outBytes.AddAll(CreateByteView(removedElementIDs));
}

template <typename HeaderType, typename IDType>
bool TryDeserializeID(const uint8_t*& deltaCompressedIter, const uint8_t* const deltaCompressedEnd, IDType& out);

template <>
bool TryDeserializeID<ECS::FullSerializedComponentHeader, uint64_t>(const uint8_t*& deltaCompressedIter, const uint8_t* const deltaCompressedEnd, uint64_t& out)
{
	const auto maybeID = Mem::LittleEndian::DeserializeUi64(deltaCompressedIter, deltaCompressedEnd);
	out = maybeID.first;
	return maybeID.second;
}

template <>
bool TryDeserializeID<ECS::FullSerializedEntityHeader, uint32_t>(const uint8_t*& deltaCompressedIter, const uint8_t* const deltaCompressedEnd, uint32_t& out)
{
	const auto maybeID = Mem::LittleEndian::DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	out = maybeID.first;
	return maybeID.second;
}

template <typename HeaderType, typename IDType>
bool TryDeltaDecompressSortedLists(
	const ECS::SerializedBytesWithViews& lastSeenBytesWithViews,
	const uint32_t numNewestElements,
	const uint8_t*& deltaCompressedIter,
	const uint8_t* const deltaCompressedEnd,
	ECS::SerializedBytesWithViews& outNewestBytesWithViews,
	Collection::Vector<IDType>& outRemovedElementIDs)
{
	const IDType invalidElementID = GetInvalidIDForHeaderType<HeaderType, IDType>();

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

	// Delta-decompress the elements.
	const auto& lastSeenBytes = lastSeenBytesWithViews.m_bytes;
	auto& outNewestBytes = outNewestBytesWithViews.m_bytes;

	const ECS::SerializedByteView* const lastSeenViewsEnd = lastSeenBytesWithViews.m_views.end();
	const ECS::SerializedByteView* lastSeenViewsIter = lastSeenBytesWithViews.m_views.begin();
	for (const auto& newestView : outNewestBytesWithViews.m_views)
	{
		// Validate the view's indices.
		if (newestView.m_beginIndex > newestView.m_endIndex)
		{
			return false;
		}

		// Read the marker and the element ID.
		auto maybeElementMarker = Mem::LittleEndian::DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
		if (!maybeElementMarker.second)
		{
			return false;
		}
		const uint32_t elementMarker = maybeElementMarker.first;

		IDType newestElementID;
		if (!TryDeserializeID<HeaderType, IDType>(deltaCompressedIter, deltaCompressedEnd, newestElementID))
		{
			return false;
		}

		// Scan forward over the last seen list while the last seen ID is less than the newest ID.
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
		}

		// Expand outNewestBytes to cover newestView.
		if (outNewestBytes.Size() < newestView.m_endIndex)
		{
			outNewestBytes.Resize(newestView.m_endIndex, 0);
		}

		// Read the element into the view.
		const uint32_t elementSizeInBytes = newestView.m_endIndex - newestView.m_beginIndex;
		Collection::ArrayView<uint8_t> rawNewestElement{ &outNewestBytes[newestView.m_beginIndex], elementSizeInBytes };
		if (elementMarker == k_elementDeltaMarker)
		{
			if (lastSeenElementID != newestElementID)
			{
				return false;
			}
			const size_t lastSeenElementSize = lastSeenViewsIter->m_endIndex - lastSeenViewsIter->m_beginIndex;
			const uint8_t* const rawLastSeenElement = &lastSeenBytes[lastSeenViewsIter->m_beginIndex];

			if (!Network::DeltaCompression::TryDecompress(
				{ rawLastSeenElement, lastSeenElementSize },
				deltaCompressedIter,
				deltaCompressedEnd,
				rawNewestElement))
			{
				return false;
			}

			// Validate the decompressed size is correct.
			if (rawNewestElement.Size() != elementSizeInBytes)
			{
				return false;
			}
		}
		else if (elementMarker == k_elementAddedMarker)
		{
			if (lastSeenElementID == newestElementID)
			{
				return false;
			}
			if ((deltaCompressedIter + elementSizeInBytes) > deltaCompressedEnd)
			{
				return false;
			}
			memcpy(rawNewestElement.begin(), deltaCompressedIter, elementSizeInBytes);
			deltaCompressedIter += elementSizeInBytes;
		}
		else
		{
			return false;
		}
	}

	// Read the list of removed elements.
	const auto maybeElementsRemovedSectionMarker =
		Mem::LittleEndian::DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	const auto maybeNumElementsRemoved = Mem::LittleEndian::DeserializeUi32(deltaCompressedIter, deltaCompressedEnd);
	if (maybeElementsRemovedSectionMarker.second == false
		|| maybeNumElementsRemoved.second == false
		|| maybeElementsRemovedSectionMarker.first != k_elementsRemovedSectionMarker)
	{
		return false;
	}

	const uint32_t numElementsRemoved = maybeNumElementsRemoved.first;
	const uint32_t sizeOfRemovedElementIDs = numElementsRemoved * sizeof(IDType);
	if ((deltaCompressedIter + sizeOfRemovedElementIDs) > deltaCompressedEnd)
	{
		return false;
	}

	outRemovedElementIDs.Resize(numElementsRemoved);
	memcpy(outRemovedElementIDs.begin(), deltaCompressedIter, sizeOfRemovedElementIDs);
	deltaCompressedIter += sizeOfRemovedElementIDs;

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
	}

	// TODO(network) handle component types that were removed entirely since the last seen frame

	// Transmit the entities.
	Mem::LittleEndian::Serialize(k_entitiesSectionMarker, outBytes);
	Mem::LittleEndian::Serialize(static_cast<uint32_t>(newestFrame.m_entities.m_views.Size()), outBytes);
	
	// Scan over the sorted entity lists to transmit the entity data.
	DeltaCompressSortedLists<ECS::FullSerializedEntityHeader, uint32_t>(
		lastSeenFrame.m_entities, newestFrame.m_entities, outBytes);
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

			// TODO(network) do something with the removed component IDs
			Collection::Vector<uint64_t> removedComponentIDs;
			if (!TryDeltaDecompressSortedLists<ECS::FullSerializedComponentHeader, uint64_t>(
				lastSeenEntryIter->second,
				numComponents,
				deltaCompressedIter,
				deltaCompressedEnd,
				outComponents,
				removedComponentIDs))
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
	
	// TODO(network) do something with the removed entity IDs
	Collection::Vector<uint32_t> removedEntityIDs;
	if (!TryDeltaDecompressSortedLists<ECS::FullSerializedEntityHeader, uint32_t>(
		lastSeenFrame.m_entities,
		numEntities,
		deltaCompressedIter,
		deltaCompressedEnd,
		outDecompressedSerialization.m_entities,
		removedEntityIDs))
	{
		return false;
	}

	return true;
}
