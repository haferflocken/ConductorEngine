#pragma once

#include <collection/VectorMap.h>
#include <unit/CountUnits.h>
#include <util/StringHash.h>

namespace ECS
{
class Component;
class ComponentID;
class ComponentInfo;
class ComponentVector;

/**
 * Creates components from component info. Component types can be registered using RegisterComponentType().
 */
class ComponentFactory
{
public:
	using FactoryFunction = bool(*)(const ComponentInfo&, const ComponentID, ComponentVector&);
	using DestructorFunction = void(*)(Component&);
	using SwapFunction = void(*)(Component&, Component&);

	struct TransmissionFunctions
	{
		using SerializeDeltaTransmissionFunction = void(*)(const Component&, const Component&, Collection::Vector<uint8_t>&);
		using SerializeFullTransmissionFunction = void(*)(const Component&, Collection::Vector<uint8_t>&);

		using ApplyDeltaTransmissionFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);
		using ApplyFullTransmissionFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);

		SerializeDeltaTransmissionFunction m_serializeDeltaTransmissionFunction;
		SerializeFullTransmissionFunction m_serializeFullTransmissionFunction;

		ApplyDeltaTransmissionFunction m_applyDeltaTransmissionFunction;
		ApplyFullTransmissionFunction m_applyFullTransmissionFunction;
	};

	ComponentFactory();

	template <typename ComponentType>
	void RegisterComponentType();

	template <typename ComponentType>
	void RegisterNetworkedComponentType();

	Unit::ByteCount64 GetSizeOfComponentInBytes(const Util::StringHash componentTypeHash) const;

	bool TryMakeComponent(const ComponentInfo& componentInfo, const ComponentID reservedID,
		ComponentVector& destination) const;

	void DestroyComponent(Component& component) const;

	void SwapComponents(Component& a, Component& b) const;

	TransmissionFunctions FindTransmissionFunctions(const Util::StringHash componentTypeHash) const;

private:
	void RegisterComponentType(const char* componentTypeName, const Util::StringHash componentTypeHash,
		const Unit::ByteCount64 sizeOfComponentInBytes, FactoryFunction factoryFn, DestructorFunction destructorFn,
		SwapFunction swapFn);

	// Maps of component type hashes to functions for those component types.
	Collection::VectorMap<Util::StringHash, Unit::ByteCount64> m_componentSizesInBytes;
	Collection::VectorMap<Util::StringHash, FactoryFunction> m_factoryFunctions;
	Collection::VectorMap<Util::StringHash, DestructorFunction> m_destructorFunctions;
	Collection::VectorMap<Util::StringHash, SwapFunction> m_swapFunctions;
	Collection::VectorMap<Util::StringHash, TransmissionFunctions> m_transmissionFunctions;
};

template <typename ComponentType>
inline void ComponentFactory::RegisterComponentType()
{
	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static bool TryCreateFromInfo(const ComponentInfo& componentInfo, const ComponentID reservedID,
			ComponentVector& destination)
		{
			return ComponentType::TryCreateFromInfo(
				static_cast<const ComponentType::Info&>(componentInfo), reservedID, destination);
		}

		static void Destroy(Component& rawComponent)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			component.~ComponentType();
		}

		static void Swap(Component& rawLHS, Component& rawRHS)
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

template <typename ComponentType>
inline void ComponentFactory::RegisterNetworkedComponentType()
{
	RegisterComponentType<ComponentType>();

	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static void SerializeDeltaTransmission(const Component& rawLHS, const Component& rawRHS,
			Collection::Vector<uint8_t>& outBytes)
		{
			const ComponentType& lhs = static_cast<const ComponentType&>(rawLHS);
			const ComponentType& rhs = static_cast<const ComponentType&>(rawRHS);

			lhs.SerializeDeltaTransmission(rhs, outBytes);
		}

		static void SerializeFullTransmission(const Component& rawComponent, Collection::Vector<uint8_t>& outBytes)
		{
			const ComponentType& component = static_cast<const ComponentType&>(rawComponent);

			lhs.SerializeTransmission(rhs);
		}

		static void ApplyDeltaTransmission(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			component.ApplyDeltaTransmission(bytes, bytesEnd);
		}

		static void ApplyFullTransmission(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			component.ApplyFullTransmission(bytes, bytesEnd);
		}
	};

	const TransmissionFunctions transmissionFunctions{
		&ComponentTypeFunctions::SerializeDeltaTransmission, &ComponentTypeFunctions::SerializeDeltaTransmission,
		&ComponentTypeFunctions::ApplyDeltaTransmission, &ComponentTypeFunctions::ApplyFullTransmission };

	m_transmissionFunctions[ComponentType::Info::sk_typeHash] = transmissionFunctions;
}
}
