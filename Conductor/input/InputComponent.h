#pragma once

#include <client/ClientID.h>
#include <collection/VectorMap.h>
#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>
#include <input/InputStateBuffer.h>

namespace Input
{
class InputComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* const sk_typeName = "input_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const override { return sk_typeHash; }

	// The named inputs this component consumes.
	Collection::Vector<Util::StringHash> m_inputNameHashes;
};

/**
 * An InputComponent makes an entity aware of user inputs. Each InputComponent can map to distinct
 * inputs from a specific client.
 */
class InputComponent final : public ECS::Component
{
public:
	using Info = InputComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const InputComponentInfo& componentInfo,
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
