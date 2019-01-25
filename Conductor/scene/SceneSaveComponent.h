#pragma once

#include <ecs/Component.h>

namespace Scene
{
/**
 * Entities with a SceneSaveComponent are saved and loaded with the scene they are a part of.
 * SceneSaveComponent is a tag component and is therefore never instantiated.
 */
class SceneSaveComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Tag;
	static constexpr const char* k_typeName = "scene_save_component";
	static const ECS::ComponentType k_type;

	SceneSaveComponent() = delete;
};
}

