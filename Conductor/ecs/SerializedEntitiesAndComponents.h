#pragma once

#include <collection/VectorMap.h>
#include <ecs/ComponentType.h>
#include <ecs/EntityFlags.h>
#include <ecs/EntityID.h>
#include <ecs/EntityLayer.h>

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
	static constexpr size_t k_unpaddedSize = 17;

	EntityID m_entityID;
	EntityID m_parentEntityID;
	uint32_t m_numComponents;
	EntityFlags m_flags;
	EntityLayer m_layer;
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
	// This is not sorted by ComponentType; the ComponentTypes match the order they were serialized in.
	Collection::Vector<Collection::Pair<ComponentType, SerializedBytesWithViews>> m_components;
	// Bytes and a list of views into them for the serialized entities, sorted by entity ID.
	// Each entity view is a FullSerializedEntityHeader followed by a list of [component type index (u16), component id (u64)].
	// Component type indices are indices into m_components so that we don't have to serialize whole type names.
	SerializedBytesWithViews m_entities;

	SerializedBytesWithViews& FindOrCreateComponentsEntry(const ComponentType componentType);
	const SerializedBytesWithViews* FindComponentsEntry(const ComponentType componentType) const;
};

/**
 * A collection of entities and components that have been removed.
 */
struct RemovedEntitiesAndComponents final
{
	Collection::VectorMap<ComponentType, Collection::Vector<uint64_t>> m_removedComponentIDs;
	Collection::Vector<uint32_t> m_removedEntityIDs;
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
	SerializedEntitiesAndComponents& outDecompressedSerialization,
	RemovedEntitiesAndComponents& outRemovedEntitiesAndComponents);
}
