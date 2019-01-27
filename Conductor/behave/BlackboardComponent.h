#pragma once

#include <behave/Blackboard.h>
#include <ecs/Component.h>

namespace Behave
{
namespace AST { class Interpreter; }

/**
 * A BlackboardComponent contains a blackboard: a data-driven key/value store.
 */
class BlackboardComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "blackboard_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const BlackboardComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(Asset::AssetManager& assetManager,
		BlackboardComponent& component,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd);

	static void BindFunctions(AST::Interpreter& interpreter);

public:
	explicit BlackboardComponent(const ECS::ComponentID id)
		: Component(id)
		, m_blackboard()
	{}

	Blackboard m_blackboard;
};
}
