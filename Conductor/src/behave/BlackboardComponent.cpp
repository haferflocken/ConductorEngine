#include <behave/BlackboardComponent.h>

#include <behave/ast/Interpreter.h>

#include <ecs/ComponentVector.h>
#include <ecs/EntityManager.h>

namespace Behave
{
const Util::StringHash BlackboardComponent::k_typeHash = Util::CalcHash(k_typeName);

void BlackboardComponent::FullySerialize(const BlackboardComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) serialize
}

void BlackboardComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	BlackboardComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	// TODO(info) apply
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

void SetBool(const ECS::Entity&, BlackboardComponent& blackboardComponent, const std::string& key, bool val)
{
	blackboardComponent.m_blackboard.Set(Util::CalcHash(key), AST::ExpressionResult::Make<bool>(val));
}

void SetNumber(const ECS::Entity&, BlackboardComponent& blackboardComponent, const std::string& key, double val)
{
	blackboardComponent.m_blackboard.Set(Util::CalcHash(key), AST::ExpressionResult::Make<double>(val));
}

void SetString(const ECS::Entity&, BlackboardComponent& blackboardComponent, const std::string& key,
	const std::string& val)
{
	blackboardComponent.m_blackboard.Set(Util::CalcHash(key), AST::ExpressionResult::Make<std::string>(val));
}

void SetComponentType(const ECS::Entity&, BlackboardComponent& blackboardComponent, const std::string& key,
	const ECS::ComponentType val)
{
	blackboardComponent.m_blackboard.Set(Util::CalcHash(key), AST::ExpressionResult::Make<ECS::ComponentType>(val));
}

void SetTreeIdentifier(const ECS::Entity&, BlackboardComponent& blackboardComponent, const std::string& key,
	const AST::TreeIdentifier val)
{
	blackboardComponent.m_blackboard.Set(Util::CalcHash(key), AST::ExpressionResult::Make<AST::TreeIdentifier>(val));
}
}

void BlackboardComponent::BindFunctions(AST::Interpreter& interpreter)
{
	interpreter.BindFunction("GetBool", &Internal_BlackboardComponent::GetBool);
	interpreter.BindFunction("GetNumber", &Internal_BlackboardComponent::GetNumber);
	interpreter.BindFunction("GetString", &Internal_BlackboardComponent::GetString);
	interpreter.BindFunction("GetComponentType", &Internal_BlackboardComponent::GetComponentType);
	interpreter.BindFunction("GetTreeIdentifier", &Internal_BlackboardComponent::GetTreeIdentifier);

	interpreter.BindFunction("SetBool", &Internal_BlackboardComponent::SetBool);
	interpreter.BindFunction("SetNumber", &Internal_BlackboardComponent::SetNumber);
	interpreter.BindFunction("SetString", &Internal_BlackboardComponent::SetString);
	interpreter.BindFunction("SetComponentType", &Internal_BlackboardComponent::SetComponentType);
	interpreter.BindFunction("SetTreeIdentifier", &Internal_BlackboardComponent::SetTreeIdentifier);
}
}
