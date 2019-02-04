#include <behave/BlackboardComponent.h>

#include <behave/ast/Interpreter.h>

#include <ecs/ComponentVector.h>
#include <ecs/EntityManager.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/InspectorInfo.h>
#include <mem/SerializeLittleEndian.h>

namespace Behave
{
const ECS::ComponentType BlackboardComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash BlackboardComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Behave::BlackboardComponent, 0);

void BlackboardComponent::FullySerialize(const BlackboardComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	const auto& map = component.m_blackboard.GetMap();

	Mem::LittleEndian::Serialize(map.Size(), outBytes);
	for (const auto& entry : map)
	{
		const Util::StringHash& key = entry.first;
		const AST::ExpressionResult& value = entry.second;

		Mem::LittleEndian::Serialize(Util::ReverseHash(key), outBytes);
		Mem::LittleEndian::Serialize(static_cast<uint32_t>(value.GetTag()), outBytes);

		value.Match(
			[](const AST::None&) {},
			[&](const bool& value)
			{
				Mem::LittleEndian::Serialize(static_cast<uint8_t>(value), outBytes);
			},
			[&](const double& value)
			{
				Mem::LittleEndian::Serialize(value, outBytes);
			},
				[&](const std::string& value)
			{
				Mem::LittleEndian::Serialize(value.c_str(), outBytes);
			},
				[&](const ECS::ComponentType& value)
			{
				const char* const typeString = Util::ReverseHash(value.GetTypeHash());
				Mem::LittleEndian::Serialize(typeString, outBytes);
			},
				[&](const AST::TreeIdentifier& value)
			{
				const char* const treeName = Util::ReverseHash(value.m_treeNameHash);
				Mem::LittleEndian::Serialize(treeName, outBytes);
			});
	}
}

void BlackboardComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	BlackboardComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	const auto maybeMapSize = Mem::LittleEndian::DeserializeUi32(bytes, bytesEnd);
	if (!maybeMapSize.second)
	{
		return;
	}
	for (size_t i = 0; i < maybeMapSize.first; ++i)
	{
		char keyBuffer[64];
		if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, keyBuffer))
		{
			return;
		}
		const auto maybeTag = Mem::LittleEndian::DeserializeUi32(bytes, bytesEnd);
		if (!maybeTag.second)
		{
			return;
		}

		// This is a hardcoded mapping of the tags to types.
		AST::ExpressionResult value;
		switch (maybeTag.first)
		{
		case 0: // None
		{
			value = AST::ExpressionResult::Make<AST::None>();
			break;
		}
		case 1: // bool
		{
			const auto maybeValue = Mem::LittleEndian::DeserializeUi8(bytes, bytesEnd);
			if (!maybeValue.second)
			{
				return;
			}
			value = AST::ExpressionResult::Make<bool>(maybeValue.first);
			break;
		}
		case 2: // double
		{
			const auto maybeValue = Mem::LittleEndian::DeserializeF64(bytes, bytesEnd);
			if (!maybeValue.second)
			{
				return;
			}
			value = AST::ExpressionResult::Make<double>(maybeValue.first);
			break;
		}
		case 3: // std::string
		{
			char valueBuffer[128];
			if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, valueBuffer))
			{
				return;
			}
			value = AST::ExpressionResult::Make<std::string>(valueBuffer);
			break;
		}
		case 4: // ComponentType
		{
			char valueBuffer[128];
			if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, valueBuffer))
			{
				return;
			}
			const Util::StringHash valueHash = Util::CalcHash(valueBuffer);
			value = AST::ExpressionResult::Make<ECS::ComponentType>(valueHash);
			break;
		}
		case 5: // TreeIdentifier
		{
			char valueBuffer[128];
			if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, valueBuffer))
			{
				return;
			}
			const Util::StringHash valueHash = Util::CalcHash(valueBuffer);
			value = AST::ExpressionResult::Make<AST::TreeIdentifier>(AST::TreeIdentifier{ valueHash });
			break;
		}
		default:
		{
			AMP_LOG_ERROR("Unrecognized tag type [%u].", maybeTag.first);
			value = AST::ExpressionResult::Make<AST::None>();
			break;
		}
		}

		const Util::StringHash keyHash = Util::CalcHash(keyBuffer);
		component.m_blackboard.Set(keyHash, value);
	}
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
