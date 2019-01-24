#include <ecs/ComponentReflector.h>

#include <ecs/Component.h>

#include <dev/Dev.h>

namespace ECS
{
ComponentReflector::ComponentReflector()
{}

void ComponentReflector::RegisterComponentType(const char* const componentTypeName,
	const Util::StringHash componentTypeHash,
	const Unit::ByteCount64 sizeOfComponent,
	const Unit::ByteCount64 alignOfComponent,
	const MandatoryComponentFunctions& mandatoryFunctions)
{
	const ComponentType componentType{ componentTypeHash };

	AMP_FATAL_ASSERT(componentTypeHash == Util::CalcHash(componentTypeName),
		"Mismatch between component type hash and component type name for component with type name \"%s\".",
		componentTypeName);

	AMP_FATAL_ASSERT(m_componentSizesInBytes.Find(componentType) == m_componentSizesInBytes.end()
		&& m_componentAlignmentsInBytes.Find(componentType) == m_componentAlignmentsInBytes.end()
		&& m_mandatoryComponentFunctions.Find(componentType) == m_mandatoryComponentFunctions.end()
		&& m_transmissionFunctions.Find(componentType) == m_transmissionFunctions.end(),
		"Attempted to register component type \"%s\", but it has already been registered.", componentTypeName);

	m_componentSizesInBytes[componentType] = sizeOfComponent;
	m_componentAlignmentsInBytes[componentType] = alignOfComponent;
	m_mandatoryComponentFunctions[componentType] = mandatoryFunctions;
}

bool ComponentReflector::IsRegistered(const ComponentType componentType) const
{
	return (m_componentSizesInBytes.Find(componentType) != m_componentSizesInBytes.end());
}

Unit::ByteCount64 ComponentReflector::GetSizeOfComponentInBytes(const ComponentType componentType) const
{
	const auto sizeItr = m_componentSizesInBytes.Find(componentType);
	if (sizeItr == m_componentSizesInBytes.end())
	{
		AMP_LOG_WARNING("Failed to find the size of component type \"%s\".",
			Util::ReverseHash(componentType.GetTypeHash()));
		return Unit::ByteCount64(0);
	}

	return sizeItr->second;
}

Unit::ByteCount64 ComponentReflector::GetAlignOfComponentInBytes(const ComponentType componentType) const
{
	const auto alignItr = m_componentAlignmentsInBytes.Find(componentType);
	if (alignItr == m_componentAlignmentsInBytes.end())
	{
		AMP_LOG_WARNING("Failed to find the align of component type \"%s\".",
			Util::ReverseHash(componentType.GetTypeHash()));
		return Unit::ByteCount64(0);
	}

	return alignItr->second;
}

Component* ComponentReflector::TryBasicConstructComponent(const ComponentID reservedID, ComponentVector& destination) const
{
	const auto iter = m_mandatoryComponentFunctions.Find(reservedID.GetType());
	if (iter == m_mandatoryComponentFunctions.end())
	{
		AMP_LOG_WARNING("Failed to find a factory function for component type \"%s\".",
			Util::ReverseHash(reservedID.GetType().GetTypeHash()));
		return nullptr;
	}

	return &iter->second.m_basicConstructFunction(reservedID, destination);
}

Component* ComponentReflector::TryMakeComponent(Asset::AssetManager& assetManager,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd,
	const ComponentID reservedID,
	ComponentVector& destination) const
{
	const auto iter = m_mandatoryComponentFunctions.Find(reservedID.GetType());
	if (iter == m_mandatoryComponentFunctions.end())
	{
		AMP_LOG_WARNING("Failed to find a factory function for component type \"%s\".",
			Util::ReverseHash(reservedID.GetType().GetTypeHash()));
		return nullptr;
	}

	Component& component = iter->second.m_basicConstructFunction(reservedID, destination);
	iter->second.m_applyFullSerializationFunction(assetManager, component, bytes, bytesEnd);
	return &component;
}

void ComponentReflector::DestroyComponent(Component& component) const
{
	const auto iter = m_mandatoryComponentFunctions.Find(component.m_id.GetType());
	AMP_FATAL_ASSERT(iter != m_mandatoryComponentFunctions.end(),
		"Failed to find a destructor function for component type \"%s\".",
		Util::ReverseHash(component.m_id.GetType().GetTypeHash()));

	iter->second.m_destructorFunction(component);
}

void ComponentReflector::SwapComponents(Component& a, Component& b) const
{
	const auto iter = m_mandatoryComponentFunctions.Find(a.m_id.GetType());
	AMP_FATAL_ASSERT(iter != m_mandatoryComponentFunctions.end()
		&& iter == m_mandatoryComponentFunctions.Find(b.m_id.GetType()),
		"Failed to find a swap function for component type \"%s\".",
		Util::ReverseHash(a.m_id.GetType().GetTypeHash()));

	iter->second.m_swapFunction(a, b);
}

bool ComponentReflector::IsNetworkedComponent(const ComponentType componentType) const
{
	return (m_transmissionFunctions.Find(componentType) != m_transmissionFunctions.end());
}

const ComponentReflector::MandatoryComponentFunctions& ECS::ComponentReflector::FindComponentFunctions(
	const ComponentType componentType) const
{
	const auto iter = m_mandatoryComponentFunctions.Find(componentType);
	AMP_FATAL_ASSERT(iter != m_mandatoryComponentFunctions.end(),
		"Failed to find mandatory component functions for component type \"%s\".",
		Util::ReverseHash(componentType.GetTypeHash()));

	return iter->second;
}

const ComponentReflector::TransmissionFunctions* ECS::ComponentReflector::FindTransmissionFunctions(
	const ComponentType componentType) const
{
	const auto transmissionItr = m_transmissionFunctions.Find(componentType);
	if (transmissionItr != m_transmissionFunctions.end())
	{
		return &transmissionItr->second;
	}
	return nullptr;
}
}
