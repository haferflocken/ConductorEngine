#pragma once

#include <client/ClientID.h>
#include <collection/VectorMap.h>
#include <ecs/Component.h>
#include <input/InputStateBuffer.h>

namespace Input
{
/**
 * An InputComponent makes an entity aware of user inputs. Each InputComponent can map to distinct
 * inputs from a specific client.
 */
class InputComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* const k_typeName = "input_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfo k_inspectorInfo;

	static void FullySerialize(const InputComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(
		Asset::AssetManager& assetManager, InputComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

public:
	explicit InputComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	// The client this InputComponent receives input from.
	Client::ClientID m_clientID;
	// Each InputComponent receives input from the keys of this map.
	Collection::VectorMap<Util::StringHash, InputStateBuffer> m_inputMap;
};
}
