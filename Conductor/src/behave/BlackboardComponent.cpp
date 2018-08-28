#include <behave/BlackboardComponent.h>

#include <behave/ast/Interpreter.h>

#include <ecs/ComponentVector.h>
#include <ecs/EntityManager.h>

namespace Behave
{
bool BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}

namespace Internal_BlackboardComponent
{
bool GetBool(const ECS::Entity&, const BlackboardComponent& blackboardComponent, const std::string& key)
{
	AST::ExpressionResult val;
	blackboardComponent.m_blackboard.GetWithDefault(Util::CalcHash(key), AST::ExpressionResult(), val);
	return val.IsAny() ? val.Get<bool>() : false;
}

double GetNumber(const ECS::Entity&, const BlackboardComponent& blackboardComponent, const std::string& key)
{
	AST::ExpressionResult val;
	blackboardComponent.m_blackboard.GetWithDefault(Util::CalcHash(key), AST::ExpressionResult(), val);
	return val.IsAny() ? val.Get<double>() : DBL_MAX;
}

std::string GetString(const ECS::Entity&, const BlackboardComponent& blackboardComponent, const std::string& key)
{
	AST::ExpressionResult val;
	blackboardComponent.m_blackboard.GetWithDefault(Util::CalcHash(key), AST::ExpressionResult(), val);
	return val.IsAny() ? val.Get<std::string>() : "ERROR: MISSING STRING";
}

ECS::ComponentType GetComponentType(const ECS::Entity&,
	const BlackboardComponent& blackboardComponent, const std::string& key)
{
	AST::ExpressionResult val;
	blackboardComponent.m_blackboard.GetWithDefault(Util::CalcHash(key), AST::ExpressionResult(), val);
	return val.IsAny() ? val.Get<ECS::ComponentType>() : ECS::ComponentType();
}

AST::TreeIdentifier GetTreeIdentifier(const ECS::Entity&,
	const BlackboardComponent& blackboardComponent, const std::string& key)
{
	AST::ExpressionResult val;
	blackboardComponent.m_blackboard.GetWithDefault(Util::CalcHash(key), AST::ExpressionResult(), val);
	return val.IsAny() ? val.Get<AST::TreeIdentifier>() : AST::TreeIdentifier();
}
}

void BlackboardComponent::BindFunctions(AST::Interpreter& interpreter)
{
	interpreter.BindFunction(Util::CalcHash("GetBool"), &Internal_BlackboardComponent::GetBool);
	interpreter.BindFunction(Util::CalcHash("GetNumber"), &Internal_BlackboardComponent::GetNumber);
	interpreter.BindFunction(Util::CalcHash("GetString"), &Internal_BlackboardComponent::GetString);
	interpreter.BindFunction(Util::CalcHash("GetComponentType"), &Internal_BlackboardComponent::GetComponentType);
	interpreter.BindFunction(Util::CalcHash("GetTreeIdentifier"), &Internal_BlackboardComponent::GetTreeIdentifier);
}
}
