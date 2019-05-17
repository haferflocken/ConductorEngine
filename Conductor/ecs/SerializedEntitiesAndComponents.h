#pragma once

#include <collection/VectorMap.h>
#include <ecs/ComponentType.h>
#include <ecs/EntityID.h>

#include <functional>
#include <iosfwd>

namespace ECS
{
struct FullSerializedComponentHeader final
{
	static constexpr size_t k_unpaddedSize = 8;

	uint64_t m_uniqueID;
};
struct FullSerializedEntityHeader final
{
	static constexpr size_t k_unpaddedSize = 12;

	EntityID m_entityID;
	EntityID m_parentEntityID;
	uint32_t m_numComponents;
};
struct SerializedByteView final
{
	uint32_t m_beginIndex;
	uint32_t m_endIndex;
};
struct SerializedBytesWithViews final
{
	Collection::Vector<uint8_t> m_bytes;
	Collection::Vector<SerializedByteView> m_views;
};

/**
 * A serialized representation of a number of entities and their components.
 */
struct SerializedEntitiesAndComponents final
{
	// Bytes and lists of views into them for serialized components of given types, sorted by component ID.
	// Each component view is a FullSerializedComponentHeader followed by the component's serialized representation.
	Collection::VectorMap<ComponentType, SerializedBytesWithViews> m_components;
	// Bytes and a list of views into them for the serialized entities, sorted by entity ID.
	// Each entity view is a FullSerializedEntityHeader followed by a list of [component type, component unique id].
	SerializedBytesWithViews m_entities;
};

// Read/write SerializedEntitiesAndComponents from/to byte representations.
void WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization,
	std::ostream& fileOutput);
void WriteSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& serialization,
	const std::function<void(const void*, size_t)>& outputFn);
bool TryReadSerializedEntitiesAndComponentsFrom(
	const Collection::ArrayView<const uint8_t> fileBytes,
	SerializedEntitiesAndComponents& outSerialization);

void DeltaCompressSerializedEntitiesAndComponentsTo(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const SerializedEntitiesAndComponents& newestFrame,
	Collection::Vector<uint8_t>& outBytes);
bool TryDeltaDecompressSerializedEntitiesAndComponentsFrom(
	const SerializedEntitiesAndComponents& lastSeenFrame,
	const Collection::ArrayView<const uint8_t> deltaCompressedBytes,
	SerializedEntitiesAndComponents& outDecompressedSerialization);
}
