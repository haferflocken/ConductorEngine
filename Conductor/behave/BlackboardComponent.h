#pragma once

#include <behave/Blackboard.h>
#include <ecs/Component.h>

namespace ECS
{
class ComponentVector;
}

namespace Behave
{
namespace AST { class Interpreter; }
class BlackboardComponentInfo;

/**
 * A BlackboardComponent contains a blackboard: a data-driven key/value store.
 */
class BlackboardComponent final : public ECS::Component
{
public:
	using Info = BlackboardComponentInfo;

	static bool TryCreateFromInfo(const BlackboardComponentInfo& componentInfo, const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	static void BindFunctions(AST::Interpreter& interpreter);

	explicit BlackboardComponent(const ECS::ComponentID id)
		: Component(id)
		, m_blackboard()
	{}

	virtual ~BlackboardComponent() {}

	Blackboard m_blackboard;
};
}
