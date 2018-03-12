#include <behave/ActorComponent.h>
#include <behave/ActorComponentFactory.h>
#include <behave/ActorComponentInfo.h>
#include <behave/components/BlackboardComponent.h>
#include <behave/components/BlackboardComponentInfo.h>
#include <behave/components/SceneTransformComponent.h>
#include <behave/components/SceneTransformComponentInfo.h>

#include <dev/Dev.h>

using namespace Behave;

namespace
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

ActorComponentFactory::ActorComponentFactory()
{
	RegisterComponentType<Components::BlackboardComponent>();
	RegisterComponentType<Components::SceneTransformComponent>();
}

void ActorComponentFactory::RegisterComponentType(const char* const componentTypeName,
	const Util::StringHash componentTypeHash, const Unit::ByteCount64 sizeOfComponentInBytes,
	FactoryFunction factoryFn, DestructorFunction destructorFn, SwapFunction swapFn)
{
	Dev::FatalAssert(componentTypeHash == Util::CalcHash(componentTypeName),
		"Mismatch between component type hash and component type name for component with type name \"s\".",
		componentTypeName);
	
	Dev::FatalAssert(m_componentSizesInBytes.find(componentTypeHash) == m_componentSizesInBytes.end()
		&& m_factoryFunctions.find(componentTypeHash) == m_factoryFunctions.end()
		&& m_destructorFunctions.find(componentTypeHash) == m_destructorFunctions.end()
		&& m_swapFunctions.find(componentTypeHash) == m_swapFunctions.end(),
		"Attempted to register component type \"%s\", but it has already been registerd.", componentTypeName);
	
	m_componentSizesInBytes[componentTypeHash] = sizeOfComponentInBytes;
	m_factoryFunctions[componentTypeHash] = std::move(factoryFn);
	m_destructorFunctions[componentTypeHash] = std::move(destructorFn);
	m_swapFunctions[componentTypeHash] = std::move(swapFn);
}

Unit::ByteCount64 ActorComponentFactory::GetSizeOfComponentInBytes(const Util::StringHash componentTypeHash) const
{
	const auto sizeItr = m_componentSizesInBytes.find(componentTypeHash);
	if (sizeItr == m_componentSizesInBytes.end())
	{
		Dev::LogWarning("Failed to find the size of component type \"%s\".", Util::ReverseHash(componentTypeHash));
		return Unit::ByteCount64(0);
	}

	return sizeItr->second;
}

bool ActorComponentFactory::TryMakeComponent(const ActorComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination) const
{
	const auto factoryItr = m_factoryFunctions.find(componentInfo.GetTypeHash());
	if (factoryItr == m_factoryFunctions.end())
	{
		Dev::LogWarning("Failed to find a factory function for component type \"%s\".", componentInfo.GetTypeName());
		return false;
	}

	return factoryItr->second(componentInfo, reservedID, destination);
}

void ActorComponentFactory::DestroyComponent(ActorComponent& component) const
{
	const auto& destructorItr = m_destructorFunctions.find(component.m_id.GetType());
	Dev::FatalAssert(destructorItr != m_destructorFunctions.end(),
		"Failed to find a destructor function for component type \"%s\".",
		Util::ReverseHash(component.m_id.GetType()));

	destructorItr->second(component);
}

void ActorComponentFactory::SwapComponents(ActorComponent& a, ActorComponent& b) const
{
	const auto& swapItr = m_swapFunctions.find(a.m_id.GetType());
	Dev::FatalAssert(swapItr != m_swapFunctions.end() && swapItr == m_swapFunctions.find(b.m_id.GetType()),
		"Failed to find a swap function for component type \"%s\".",
		Util::ReverseHash(a.m_id.GetType()));

	swapItr->second(a, b);
}
