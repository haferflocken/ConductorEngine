#pragma once

#include <ecs/ComponentType.h>

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
 * Reflects functions of components using templates and storing function pointers. This is used to provide
 * more type safety and more flexible interfaces for components than is straightforwardly possible with virtual
 * function interfaces.
 * Component types can be registered using RegisterComponentType().
 */
class ComponentReflector final
{
public:
	using FactoryFunction = bool(*)(const ComponentInfo&, const ComponentID, ComponentVector&);
	using DestructorFunction = void(*)(Component&);
	using SwapFunction = void(*)(Component&, Component&);

	struct TransmissionFunctions
	{
		using TryCreateFromTransmissionFunction = bool(*)(const uint8_t*&, const uint8_t*, const ComponentID, ComponentVector&);
		using CopyConstructFunction = void(*)(void*, const Component&);

		using SerializeDeltaTransmissionFunction = void(*)(const Component&, const Component&, Collection::Vector<uint8_t>&);
		using SerializeFullTransmissionFunction = void(*)(const Component&, Collection::Vector<uint8_t>&);

		using ApplyDeltaTransmissionFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);
		using ApplyFullTransmissionFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);

		TryCreateFromTransmissionFunction m_tryCreateFromTransmissionFunction;
		CopyConstructFunction m_copyConstructFunction;

		SerializeDeltaTransmissionFunction m_serializeDeltaTransmissionFunction;
		SerializeFullTransmissionFunction m_serializeFullTransmissionFunction;

		ApplyDeltaTransmissionFunction m_applyDeltaTransmissionFunction;
		ApplyFullTransmissionFunction m_applyFullTransmissionFunction;
	};

	ComponentReflector();
	
	// Register a component type that does not support network transmission.
	template <typename ComponentType>
	void RegisterComponentType();

	// Register a component type that supports network transmission using static functions defined in ComponentType.
	template <typename ComponentType>
	void RegisterNetworkedComponentType();

	// Register a component type that supports network transmission using memory mirroring. This is fine for
	// many varieties of components, but only works so long as the client and host have the same endianess.
	template <typename ComponentType>
	void RegisterMemoryImagedNetworkedComponentType();

	Unit::ByteCount64 GetSizeOfComponentInBytes(const ComponentType componentType) const;

	bool TryMakeComponent(const ComponentInfo& componentInfo, const ComponentID reservedID,
		ComponentVector& destination) const;
	void DestroyComponent(Component& component) const;
	void SwapComponents(Component& a, Component& b) const;

	bool IsNetworkedComponent(const ComponentType componentType) const;

	DestructorFunction FindDestructorFunction(const ComponentType componentType) const;
	const TransmissionFunctions* FindTransmissionFunctions(const ComponentType componentType) const;

private:
	void RegisterComponentType(const char* componentTypeName, const Util::StringHash componentTypeHash,
		const Unit::ByteCount64 sizeOfComponentInBytes, FactoryFunction factoryFn, DestructorFunction destructorFn,
		SwapFunction swapFn);

	// Maps of component types to functions for those component types.
	Collection::VectorMap<ComponentType, Unit::ByteCount64> m_componentSizesInBytes;
	Collection::VectorMap<ComponentType, FactoryFunction> m_factoryFunctions;
	Collection::VectorMap<ComponentType, DestructorFunction> m_destructorFunctions;
	Collection::VectorMap<ComponentType, SwapFunction> m_swapFunctions;
	Collection::VectorMap<ComponentType, TransmissionFunctions> m_transmissionFunctions;
};

template <typename ComponentType>
inline void ComponentReflector::RegisterComponentType()
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
inline void ComponentReflector::RegisterNetworkedComponentType()
{
	RegisterComponentType<ComponentType>();

	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static bool TryCreateFromTransmission(const uint8_t*& bytes, const uint8_t* bytesEnd,
			const ComponentID reservedID, ComponentVector& destination)
		{
			return ComponentType::TryCreateFromTransmission(bytes, bytesEnd, reservedID, destination);
		}

		static void CopyConstruct(void* place, const Component& rawSource)
		{
			const ComponentType& sourceComponent = static_cast<const ComponentType&>(rawSource);
			new (place) ComponentType(sourceComponent);
		}

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
		&ComponentTypeFunctions::TryCreateFromTransmission, &ComponentTypeFunctions::CopyConstruct,
		&ComponentTypeFunctions::SerializeDeltaTransmission, &ComponentTypeFunctions::SerializeDeltaTransmission,
		&ComponentTypeFunctions::ApplyDeltaTransmission, &ComponentTypeFunctions::ApplyFullTransmission };

	m_transmissionFunctions[ComponentType::Info::sk_typeHash] = transmissionFunctions;
}


template <typename ComponentType>
inline void ComponentReflector::RegisterMemoryImagedNetworkedComponentType()
{
	RegisterComponentType<ComponentType>();

	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static bool TryCreateFromTransmission(const uint8_t*& bytes, const uint8_t* bytesEnd,
			const ComponentID reservedID, ComponentVector& destination)
		{
			if ((bytes + sizeof(ComponentType) - 9) >= bytesEnd)
			{
				bytes = bytesEnd;
				return false;
			}

			ComponentType& component = destination.Emplace<ComponentType>(reservedID);
			ApplyFullTransmission(component, bytes, bytesEnd);
			return true;
		}

		static void CopyConstruct(void* place, const Component& rawSource)
		{
			const ComponentType& sourceComponent = static_cast<const ComponentType&>(rawSource);
			new (place) ComponentType(sourceComponent);
		}

		static void SerializeDeltaTransmission(const Component& rawLHS, const Component& rawRHS,
			Collection::Vector<uint8_t>& outBytes)
		{
			SerializeFullTransmission(rawRHS, outBytes);
		}

		static void SerializeFullTransmission(const Component& rawComponent, Collection::Vector<uint8_t>& outBytes)
		{
			const ComponentType& component = static_cast<const ComponentType&>(rawComponent);
			const uint8_t* componentBytes = reinterpret_cast<const uint8_t*>(&component);
			componentBytes += 8; // Skip the vtable pointer.
			for (size_t i = 0; i < (sizeof(ComponentType) - 8); ++i)
			{
				outBytes.Add(componentBytes);
			}
		}

		static void ApplyDeltaTransmission(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			ApplyFullTransmission(rawComponent, bytes, bytesEnd);
		}

		static void ApplyFullTransmission(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			if ((bytes + sizeof(ComponentType) - 9) >= bytesEnd)
			{
				bytes = bytesEnd;
				return;
			}

			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			uint8_t* componentBytes = reinterpret_cast<uint8_t*>(&component);
			componentBytes += 8; // Skip the vtable pointer.
			for (size_t i = 0; i < (sizeof(ComponentType) - 8); ++i)
			{
				componentBytes[i] = *(bytes++);
			}
		}
	};

	const TransmissionFunctions transmissionFunctions{
		&ComponentTypeFunctions::TryCreateFromTransmission, &ComponentTypeFunctions::CopyConstruct,
		&ComponentTypeFunctions::SerializeDeltaTransmission, &ComponentTypeFunctions::SerializeDeltaTransmission,
		&ComponentTypeFunctions::ApplyDeltaTransmission, &ComponentTypeFunctions::ApplyFullTransmission };

	m_transmissionFunctions[ComponentType::Info::sk_typeHash] = transmissionFunctions;
}
}
