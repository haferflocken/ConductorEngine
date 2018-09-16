#pragma once

#include <ecs/ComponentID.h>
#include <json/JSONTypes.h>

namespace Asset { class AssetManager; }

namespace ECS
{
class ComponentVector;

/**
 * A Component holds data for an entity. A component is always instantiated using a ComponentInfo.
 * Components must define an Info type which they are instantiated from, and must define a TryCreateFromInfo
 * static function which creates them from their info.
 */
class Component
{
public:
	explicit Component(const ComponentID id)
		: m_id(id)
	{}

	virtual ~Component() {}

	// Save() must generate a JSONObject which can be used to restore the state of the component using Load(...).
	// The JSONObject returned by Save() must have a "type" string field.
	virtual JSON::JSONObject Save() const { return JSON::JSONObject(); }
	virtual void Load(const JSON::JSONObject& jsonData) {}

	ComponentID m_id;
};
}
