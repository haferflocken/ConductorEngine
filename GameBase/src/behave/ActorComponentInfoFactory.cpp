#include <behave/ActorComponentInfo.h>
#include <behave/ActorComponentInfoFactory.h>
#include <behave/components/BlackboardComponentInfo.h>
#include <behave/components/SceneTransformComponentInfo.h>

#include <dev/Dev.h>
#include <json/JSONTypes.h>

using namespace Behave;

namespace
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

ActorComponentInfoFactory::ActorComponentInfoFactory()
	: m_factoryFunctions()
{
	RegisterFactoryFunction<Components::BlackboardComponentInfo>();
	RegisterFactoryFunction<Components::SceneTransformComponentInfo>();
}

void ActorComponentInfoFactory::RegisterFactoryFunction(const char* const componentTypeName, FactoryFunction fn)
{
	const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeName);
	Dev::FatalAssert(m_factoryFunctions.find(componentTypeHash) == m_factoryFunctions.end(),
		"Attempted to register a factory function for component type \"%s\", but there already is one.");
	m_factoryFunctions[componentTypeHash] = std::move(fn);
}

Mem::UniquePtr<ActorComponentInfo> ActorComponentInfoFactory::MakeComponentInfo(
	const JSON::JSONObject& jsonObject) const
{
	const JSON::JSONString* const jsonTypeName = jsonObject.FindString(k_typeKeyHash);
	if (jsonTypeName == nullptr)
	{
		Dev::LogWarning("Failed to find a type name.");
		return nullptr;
	}

	const auto factoryItr = m_factoryFunctions.find(jsonTypeName->m_hash);
	if (factoryItr == m_factoryFunctions.end())
	{
		Dev::LogWarning("Failed to find a factory function for component type \"%s\".",
			jsonTypeName->m_string.c_str());
		return nullptr;
	}

	return factoryItr->second(jsonObject);
}
