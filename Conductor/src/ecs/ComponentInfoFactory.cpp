#include <ecs/ComponentInfo.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/components/BehaviourTreeComponentInfo.h>
#include <ecs/components/BlackboardComponentInfo.h>
#include <ecs/components/SceneTransformComponentInfo.h>

#include <dev/Dev.h>
#include <json/JSONTypes.h>

namespace Internal_ActorComponentInfoFactory
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

ECS::ComponentInfoFactory::ComponentInfoFactory()
	: m_factoryFunctions()
{
	RegisterFactoryFunction<Components::BehaviourTreeComponentInfo>();
	RegisterFactoryFunction<Components::BlackboardComponentInfo>();
	RegisterFactoryFunction<Components::SceneTransformComponentInfo>();
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
		jsonObject.FindString(Internal_ActorComponentInfoFactory::k_typeKeyHash);
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
