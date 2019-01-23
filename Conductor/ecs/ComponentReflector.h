#pragma once

#include <ecs/ComponentType.h>

#include <collection/VectorMap.h>
#include <unit/CountUnits.h>
#include <util/StringHash.h>

namespace Asset { class AssetManager; }

namespace ECS
{
class Component;
class ComponentID;
class ComponentVector;

/**
 * Reflects functions of components using templates and storing function pointers. This is used to provide
 * more type safety and more flexible interfaces for components than is straightforwardly possible with virtual
 * function interfaces. Additionally, it removes the need for components to have a vtable pointer.
 * Component types can be registered using RegisterComponentType().
 */
class ComponentReflector final
{
public:
	using BasicConstructFunction = void(*)(const ComponentID, ComponentVector&);
	using TryCreateFromFullSerializationFunction = bool(*)(Asset::AssetManager&, const uint8_t*&, const uint8_t*, const ComponentID, ComponentVector&);
	using FullSerializationFunction = void(*)(const Component&, Collection::Vector<uint8_t>&);
	using ApplyFullSerializationFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);
	using DestructorFunction = void(*)(Component&);
	using SwapFunction = void(*)(Component&, Component&);

	struct MandatoryComponentFunctions
	{
		BasicConstructFunction m_basicConstructFunction;
		TryCreateFromFullSerializationFunction m_tryCreateFromFullSerializationFunction;
		FullSerializationFunction m_fullSerializationFunction;
		ApplyFullSerializationFunction m_applyFullSerializationFunction;
		DestructorFunction m_destructorFunction;
		SwapFunction m_swapFunction;
	};

	struct TransmissionFunctions
	{
		using CopyConstructFunction = void(*)(void*, const Component&);
		using SerializeDeltaTransmissionFunction = void(*)(const Component&, const Component&, Collection::Vector<uint8_t>&);
		using ApplyDeltaTransmissionFunction = void(*)(Component&, const uint8_t*&, const uint8_t*);

		CopyConstructFunction m_copyConstructFunction;
		SerializeDeltaTransmissionFunction m_serializeDeltaTransmissionFunction;
		ApplyDeltaTransmissionFunction m_applyDeltaTransmissionFunction;
	};

	ComponentReflector();

	template <typename ComponentType>
	void RegisterComponentType();

	bool IsRegistered(const ComponentType componentType) const;

	Unit::ByteCount64 GetSizeOfComponentInBytes(const ComponentType componentType) const;
	Unit::ByteCount64 GetAlignOfComponentInBytes(const ComponentType componentType) const;

	bool TryBasicConstructComponent(const ComponentID reservedID, ComponentVector& destination) const;
	bool TryMakeComponent(Asset::AssetManager& assetManager,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd,
		const ComponentID reservedID,
		ComponentVector& destination) const;
	void DestroyComponent(Component& component) const;
	void SwapComponents(Component& a, Component& b) const;

	bool IsNetworkedComponent(const ComponentType componentType) const;

	const MandatoryComponentFunctions& FindComponentFunctions(const ComponentType componentType) const;
	const TransmissionFunctions* FindTransmissionFunctions(const ComponentType componentType) const;

private:
	// Register a component type that doesn't support network transmission. The component type must define:
	//   A unary constructor which takes a ComponentID.
	//   static bool TryCreateFromFullSerialization(...): creates the component from a serialized representation.
	//   static void FullySerialize(...) : serializes a complete representation of the component.
	//   static void ApplyFullSerialization(...) : applies a serialized representation of the component to it.
	template <typename ComponentType>
	void RegisterNormalComponentType();

	// Registers a component type that can't be instantiated and doesn't support network transmission.
	// These components only exist as "tags" for entities to be matched with in ECS::Systems.
	template <typename ComponentType>
	void RegisterTagComponentType();

	// Register a component type that doesn't support network transmission, automatically defining serialization and
	// deserialization functions using memcpy. The component must define a unary constructor which takes a ComponentID.
	template <typename ComponentType>
	void RegisterMemoryImagedComponentType();

	// Register a component type that supports network transmission using static functions defined in ComponentType.
	// It must meet all of the requirements of RegisterNormalComponentType(...) and must also define:
	//   A copy constructor.
	//   void SerializeDeltaTransmission(...): compares the component to its past copy and serializes a delta.
	//   void ApplyDeltaTransmission(...): applies a delta serialization to update the component.
	template <typename ComponentType>
	void RegisterNetworkedComponentType();

	void RegisterComponentType(const char* componentTypeName,
		const Util::StringHash componentTypeHash,
		const Unit::ByteCount64 sizeOfComponent,
		const Unit::ByteCount64 alignOfComponent,
		const MandatoryComponentFunctions& componentFunctions);

	// Maps of component types to functions for those component types.
	Collection::VectorMap<ComponentType, Unit::ByteCount64> m_componentSizesInBytes;
	Collection::VectorMap<ComponentType, Unit::ByteCount64> m_componentAlignmentsInBytes;
	Collection::VectorMap<ComponentType, MandatoryComponentFunctions> m_mandatoryComponentFunctions;
	Collection::VectorMap<ComponentType, TransmissionFunctions> m_transmissionFunctions;
};
}

// Inline implementations.
namespace ECS
{
template <typename ComponentType>
inline void ComponentReflector::RegisterComponentType()
{
	if constexpr (ComponentType::k_bindingType == ComponentBindingType::Normal)
	{
		RegisterNormalComponentType<ComponentType>();
	}
	else if constexpr (ComponentType::k_bindingType == ComponentBindingType::Tag)
	{
		RegisterTagComponentType<ComponentType>();
	}
	else if constexpr (ComponentType::k_bindingType == ComponentBindingType::MemoryImaged)
	{
		RegisterMemoryImagedComponentType<ComponentType>();
	}
	else if constexpr (ComponentType::k_bindingType == ComponentBindingType::Networked)
	{
		RegisterNetworkedComponentType<ComponentType>();
	}
}

template <typename ComponentType>
inline void ComponentReflector::RegisterNormalComponentType()
{
	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static void BasicConstruct(const ComponentID reservedID, ComponentVector& destination)
		{
			destination.Emplace<ComponentType>(reservedID);
		}

		static bool TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
			const uint8_t*& bytes,
			const uint8_t* bytesEnd,
			const ComponentID reservedID,
			ComponentVector& destination)
		{
			return ComponentType::TryCreateFromFullSerialization(
				assetManager, bytes, bytesEnd, reservedID, destination);
		}

		static void FullySerialize(const Component& rawComponent, Collection::Vector<uint8_t>& outBytes)
		{
			const ComponentType& component = static_cast<const ComponentType&>(rawComponent);
			ComponentType::FullySerialize(component, outBytes);
		}

		static void ApplyFullSerialization(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			ComponentType::ApplyFullSerialization(component, bytes, bytesEnd);
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

	MandatoryComponentFunctions functions{ &ComponentTypeFunctions::BasicConstruct,
		&ComponentTypeFunctions::TryCreateFromFullSerialization,
		&ComponentTypeFunctions::FullySerialize,
		&ComponentTypeFunctions::ApplyFullSerialization,
		&ComponentTypeFunctions::Destroy,
		&ComponentTypeFunctions::Swap };

	RegisterComponentType(ComponentType::k_typeName,
		ComponentType::k_typeHash,
		Unit::ByteCount64(sizeof(ComponentType)),
		Unit::ByteCount64(alignof(ComponentType)),
		functions);
}

template <typename ComponentType>
inline void ComponentReflector::RegisterTagComponentType()
{
	MandatoryComponentFunctions functions{ nullptr, nullptr, nullptr, nullptr, nullptr };

	RegisterComponentType(ComponentType::k_typeName,
		ComponentType::k_typeHash,
		Unit::ByteCount64(0),
		Unit::ByteCount64(0),
		functions);
}

template <typename ComponentType>
inline void ComponentReflector::RegisterMemoryImagedComponentType()
{
	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
		static void BasicConstruct(const ComponentID reservedID, ComponentVector& destination)
		{
			destination.Emplace<ComponentType>(reservedID);
		}

		static bool TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
			const uint8_t*& bytes,
			const uint8_t* bytesEnd,
			const ComponentID reservedID,
			ComponentVector& destination)
		{
			if ((bytesEnd - bytes) < sizeof(ComponentType))
			{
				return false;
			}
			ComponentType& component = destination.Emplace<ComponentType>(reservedID);
			memcpy(&component, bytes, sizeof(ComponentType));
			component.m_id = reservedID;
			bytes += sizeof(ComponentType);
			return true;
		}

		static void FullySerialize(const Component& rawComponent, Collection::Vector<uint8_t>& outBytes)
		{
			const ComponentType& component = static_cast<const ComponentType&>(rawComponent);
			const uint8_t* const componentBytes = reinterpret_cast<const uint8_t*>(&component);
			outBytes.AddAll({ componentBytes, sizeof(ComponentType) });
		}

		static void ApplyFullSerialization(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			if ((bytesEnd - bytes) >= sizeof(ComponentType))
			{
				ComponentType& component = static_cast<ComponentType&>(rawComponent);
				memcpy(&component, bytes, sizeof(ComponentType));
				bytes += sizeof(ComponentType);
			}
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

	MandatoryComponentFunctions functions{ &ComponentTypeFunctions::BasicConstruct,
		&ComponentTypeFunctions::TryCreateFromFullSerialization,
		&ComponentTypeFunctions::FullySerialize,
		&ComponentTypeFunctions::ApplyFullSerialization,
		&ComponentTypeFunctions::Destroy,
		&ComponentTypeFunctions::Swap };

	RegisterComponentType(ComponentType::k_typeName,
		ComponentType::k_typeHash,
		Unit::ByteCount64(sizeof(ComponentType)),
		Unit::ByteCount64(alignof(ComponentType)),
		functions);
}

template <typename ComponentType>
inline void ComponentReflector::RegisterNetworkedComponentType()
{
	RegisterComponentType<ComponentType>();

	// Utilize the type to handle as much boilerplate casting and definition as possible.
	struct ComponentTypeFunctions
	{
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

		static void ApplyDeltaTransmission(Component& rawComponent, const uint8_t*& bytes, const uint8_t* bytesEnd)
		{
			ComponentType& component = static_cast<ComponentType&>(rawComponent);
			component.ApplyDeltaTransmission(bytes, bytesEnd);
		}
	};

	const TransmissionFunctions transmissionFunctions{ &ComponentTypeFunctions::CopyConstruct,
		&ComponentTypeFunctions::SerializeDeltaTransmission,
		&ComponentTypeFunctions::ApplyDeltaTransmission };

	m_transmissionFunctions[ComponentType::k_typeHash] = transmissionFunctions;
}
}
