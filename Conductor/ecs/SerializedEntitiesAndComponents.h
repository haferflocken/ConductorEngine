#pragma once

#include <collection/VectorMap.h>
#include <ecs/ComponentType.h>
#include <ecs/EntityID.h>

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

/**
 * A serialized representation of a number of entities and their components.
 */
struct SerializedEntitiesAndComponents final
{
	// The memory the serialized entities and their components are stored in.
	Collection::Vector<uint8_t> m_bytes;
	// Lists of views into m_bytes for serialized components of given types.
	// Each component view is a FullSerializedComponentHeader followed by the number of bytes specified in the header.
	Collection::VectorMap<ComponentType, Collection::Vector<SerializedByteView>> m_componentViews;
	// A list of views into m_bytes for the serialized entities.
	// Each entity view is a FullSerializedEntityHeader followed by a list of [component type, component unique id].
	Collection::Vector<SerializedByteView> m_entityViews;
};

// Read/write SerializedEntitiesAndComponents from/to files.
void WriteSerializedEntitiesAndComponentsToFile(
	const SerializedEntitiesAndComponents& serialization, std::ofstream& fileOutput);
bool TryReadSerializedEntitiesAndComponentsFromFile(
	Collection::ArrayView<const uint8_t> fileBytes,
	SerializedEntitiesAndComponents& serialization);
}
