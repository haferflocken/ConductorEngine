#pragma once

#include <behave/conditionast/ASTTypes.h>

namespace ECS { class Entity; }

namespace Behave
{
namespace ConditionAST { class Interpreter; }

class BehaviourCondition
{
public:
	BehaviourCondition(ConditionAST::Expression&& expression);

	bool Check(const ConditionAST::Interpreter& interpreter, const ECS::Entity& entity) const;

private:
	ConditionAST::Expression m_expression;
};
}
