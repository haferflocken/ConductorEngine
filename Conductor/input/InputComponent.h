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
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit InputComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	// The client this InputComponent receives input from.
	Client::ClientID m_clientID;
	// Each InputComponent receives input from the keys of this map.
	Collection::VectorMap<Util::StringHash, InputStateBuffer> m_inputMap;
};
}
