#include <ecs/ComponentInfoFactory.h>

#include <behave/BehaviourTreeComponentInfo.h>
#include <behave/BlackboardComponentInfo.h>
#include <dev/Dev.h>
#include <ecs/ComponentInfo.h>
#include <json/JSONTypes.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Internal_ComponentInfoFactory
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

ECS::ComponentInfoFactory::ComponentInfoFactory()
	: m_factoryFunctions()
{
	RegisterFactoryFunction<Behave::BehaviourTreeComponentInfo>();
	RegisterFactoryFunction<Behave::BlackboardComponentInfo>();
	RegisterFactoryFunction<Scene::SceneTransformComponentInfo>();
}

void ECS::ComponentInfoFactory::RegisterFactoryFunction(
	const char* const componentTypeName, FactoryFunction fn)
{
	const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeName);
	Dev::FatalAssert(m_factoryFunctions.Find(componentTypeHash) == m_factoryFunctions.end(),
		"Attempted to register a factory function for component type \"%s\", but there already is one.");
	m_factoryFunctions[componentTypeHash] = std::move(fn);
}

Mem::UniquePtr<ECS::ComponentInfo> ECS::ComponentInfoFactory::MakeComponentInfo(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject) const
{
	const JSON::JSONString* const jsonTypeName =
		jsonObject.FindString(Internal_ComponentInfoFactory::k_typeKeyHash);
	if (jsonTypeName == nullptr)
	{
		Dev::LogWarning("Failed to find a type name.");
		return nullptr;
	}

	const auto factoryItr = m_factoryFunctions.Find(jsonTypeName->m_hash);
	if (factoryItr == m_factoryFunctions.end())
	{
		Dev::LogWarning("Failed to find a factory function for component type \"%s\".",
			jsonTypeName->m_string.c_str());
		return nullptr;
	}

	return factoryItr->second(behaviourTreeManager, jsonObject);
}
