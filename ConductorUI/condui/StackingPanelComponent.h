#pragma once

#include <ecs/Component.h>

namespace Condui
{
/**
 * An entity with a StackingPanelComponent dynamically positions its children as a vertical stack.
 * StackingPanelComponent is a tag component and is therefore never instantiated.
 */
class StackingPanelComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Tag;
	static constexpr const char* k_typeName = "stacking_panel_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	StackingPanelComponent() = delete;
};
}
