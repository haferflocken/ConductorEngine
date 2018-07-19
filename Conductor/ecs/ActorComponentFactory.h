#pragma once

#include <collection/VectorMap.h>
#include <unit/CountUnits.h>
#include <util/StringHash.h>

namespace ECS
{
class ActorComponentID;
class ActorComponentInfo;
class ActorComponentVector;

/**
 * Creates actor components from actor component info. Component types can be registered using RegisterComponentType().
 */
class ActorComponentFactory
{
public:
	using FactoryFunction = bool(*)(const ActorComponentInfo&, const ActorComponentID, ActorComponentVector&);
	using DestructorFunction = void(*)(ActorComponent&);
	using SwapFunction = void(*)(ActorComponent&, ActorComponent&);

	ActorComponentFactory();

	template <typename ComponentType>
	void RegisterComponentType();

	Unit::ByteCount64 GetSizeOfComponentInBytes(const Util::StringHash componentTypeHash) const;

	bool TryMakeComponent(const ActorComponentInfo& componentInfo, const ActorComponentID reservedID,
		ActorComponentVector& destination) const;

	void DestroyComponent(ActorComponent& component) const;

	void SwapComponents(ActorComponent& a, ActorComponent& b) const;

private:
	void RegisterComponentType(const char* componentTypeName, const Util::StringHash componentTypeHash,
		const Unit::ByteCount64 sizeOfComponentInBytes, FactoryFunction factoryFn, DestructorFunction destructorFn,
		SwapFunction swapFn);

	// Maps component type hashes to the size in bytes of those components.
	Collection::VectorMap<Util::StringHash, Unit::ByteCount64> m_componentSizesInBytes;

	// Maps component type hashes to factory functions for those component types.
	Collection::VectorMap<Util::StringHash, FactoryFunction> m_factoryFunctions;

	// Maps component type hashes to destructor functions for those component types.
	Collection::VectorMap<Util::StringHash, DestructorFunction> m_destructorFunctions;

	// Maps component type hashes to swap functions for those component types.
	Collection::VectorMap<Util::StringHash, SwapFunction> m_swapFunctions;
};

template <typename ComponentType>
void ActorComponentFactory::RegisterComponentType()
{
	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static bool TryCreateFromInfo(const ActorComponentInfo& componentInfo, const ActorComponentID reservedID,
			ActorComponentVector& destination)
		{
			return ComponentType::TryCreateFromInfo(
				static_cast<const ComponentType::Info&>(componentInfo), reservedID, destination);
		}

		static void Destroy(ActorComponent& rawComponent)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			component.~ComponentType();
		}

		static void Swap(ActorComponent& rawLHS, ActorComponent& rawRHS)
		{
			ComponentType& lhs = static_cast<ComponentType&>(rawLHS);
			ComponentType& rhs = static_cast<ComponentType&>(rawRHS);

			ComponentType buffer = std::move(lhs);
			lhs = std::move(rhs);
			rhs = std::move(buffer);
		}
	};

	RegisterComponentType(ComponentType::Info::sk_typeName, ComponentType::Info::sk_typeHash,
		Unit::ByteCount64(sizeof(ComponentType)), &ComponentTypeFunctions::TryCreateFromInfo,
		&ComponentTypeFunctions::Destroy, &ComponentTypeFunctions::Swap);
}
}
