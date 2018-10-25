#include <ecs/ComponentReflector.h>

#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>

#include <dev/Dev.h>

ECS::ComponentReflector::ComponentReflector()
{}

void ECS::ComponentReflector::RegisterComponentType(const char* const componentTypeName,
	const Util::StringHash componentTypeHash, const Unit::ByteCount64 sizeOfComponent,
	const Unit::ByteCount64 alignOfComponent,
	FactoryFunction factoryFn, DestructorFunction destructorFn, SwapFunction swapFn)
{
	const ComponentType componentType{ componentTypeHash };

	AMP_FATAL_ASSERT(componentTypeHash == Util::CalcHash(componentTypeName),
		"Mismatch between component type hash and component type name for component with type name \"s\".",
		componentTypeName);
	
	AMP_FATAL_ASSERT(m_componentSizesInBytes.Find(componentType) == m_componentSizesInBytes.end()
		&& m_componentAlignmentsInBytes.Find(componentType) == m_componentAlignmentsInBytes.end()
		&& m_factoryFunctions.Find(componentType) == m_factoryFunctions.end()
		&& m_destructorFunctions.Find(componentType) == m_destructorFunctions.end()
		&& m_swapFunctions.Find(componentType) == m_swapFunctions.end()
		&& m_transmissionFunctions.Find(componentType) == m_transmissionFunctions.end(),
		"Attempted to register component type \"%s\", but it has already been registered.", componentTypeName);
	
	m_componentSizesInBytes[componentType] = sizeOfComponent;
	m_componentAlignmentsInBytes[componentType] = alignOfComponent;
	m_factoryFunctions[componentType] = factoryFn;
	m_destructorFunctions[componentType] = destructorFn;
	m_swapFunctions[componentType] = swapFn;
}

bool ECS::ComponentReflector::IsRegistered(const ComponentType componentType) const
{
	return (m_componentSizesInBytes.Find(componentType) != m_componentSizesInBytes.end());
}

Unit::ByteCount64 ECS::ComponentReflector::GetSizeOfComponentInBytes(const ComponentType componentType) const
{
	const auto sizeItr = m_componentSizesInBytes.Find(componentType);
	if (sizeItr == m_componentSizesInBytes.end())
	{
		Dev::LogWarning("Failed to find the size of component type \"%s\".",
			Util::ReverseHash(componentType.GetTypeHash()));
		return Unit::ByteCount64(0);
	}

	return sizeItr->second;
}

Unit::ByteCount64 ECS::ComponentReflector::GetAlignOfComponentInBytes(const ComponentType componentType) const
{
	const auto alignItr = m_componentAlignmentsInBytes.Find(componentType);
	if (alignItr == m_componentAlignmentsInBytes.end())
	{
		Dev::LogWarning("Failed to find the align of component type \"%s\".",
			Util::ReverseHash(componentType.GetTypeHash()));
		return Unit::ByteCount64(0);
	}

	return alignItr->second;
}

bool ECS::ComponentReflector::TryMakeComponent(Asset::AssetManager& assetManager, const ComponentInfo& componentInfo,
	const ComponentID reservedID, ComponentVector& destination) const
{
	const auto factoryItr = m_factoryFunctions.Find(ComponentType(componentInfo.GetTypeHash()));
	if (factoryItr == m_factoryFunctions.end())
	{
		Dev::LogWarning("Failed to find a factory function for component type \"%s\".", componentInfo.GetTypeName());
		return false;
	}

	return factoryItr->second(assetManager, componentInfo, reservedID, destination);
}

void ECS::ComponentReflector::DestroyComponent(Component& component) const
{
	const auto& destructorItr = m_destructorFunctions.Find(component.m_id.GetType());
	AMP_FATAL_ASSERT(destructorItr != m_destructorFunctions.end(),
		"Failed to find a destructor function for component type \"%s\".",
		Util::ReverseHash(component.m_id.GetType().GetTypeHash()));

	destructorItr->second(component);
}

void ECS::ComponentReflector::SwapComponents(Component& a, Component& b) const
{
	const auto& swapItr = m_swapFunctions.Find(a.m_id.GetType());
	AMP_FATAL_ASSERT(swapItr != m_swapFunctions.end() && swapItr == m_swapFunctions.Find(b.m_id.GetType()),
		"Failed to find a swap function for component type \"%s\".",
		Util::ReverseHash(a.m_id.GetType().GetTypeHash()));

	swapItr->second(a, b);
}

bool ECS::ComponentReflector::IsNetworkedComponent(const ComponentType componentType) const
{
	return (m_transmissionFunctions.Find(componentType) != m_transmissionFunctions.end());
}

ECS::ComponentReflector::DestructorFunction ECS::ComponentReflector::FindDestructorFunction(
	const ComponentType componentType) const
{
	const auto& destructorItr = m_destructorFunctions.Find(componentType);
	AMP_FATAL_ASSERT(destructorItr != m_destructorFunctions.end(),
		"Failed to find destructor function for component type \"%s\".",
		Util::ReverseHash(componentType.GetTypeHash()));

	return destructorItr->second;
}

const ECS::ComponentReflector::TransmissionFunctions* ECS::ComponentReflector::FindTransmissionFunctions(
	const ComponentType componentType) const
{
	const auto transmissionItr = m_transmissionFunctions.Find(componentType);
	if (transmissionItr != m_transmissionFunctions.end())
	{
		return &transmissionItr->second;
	}
	return nullptr;
}
