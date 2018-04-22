#pragma once

#include <collection/VectorMap.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace JSON { class JSONObject; }

namespace Behave
{
class ActorComponentInfo;
class BehaviourTreeManager;

/**
 * Creates actor component info structs from JSON files.
 */
class ActorComponentInfoFactory
{
public:
	using FactoryFunction = Mem::UniquePtr<ActorComponentInfo>(*)(const BehaviourTreeManager&, const JSON::JSONObject&);

	ActorComponentInfoFactory();

	template <typename ComponentInfoType>
	void RegisterFactoryFunction();

	Mem::UniquePtr<ActorComponentInfo> MakeComponentInfo(
		const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject) const;

private:
	void RegisterFactoryFunction(const char* const componentTypeName, FactoryFunction fn);
	
	// Maps hashes of component type names to factory functions for those component types.
	Collection::VectorMap<Util::StringHash, FactoryFunction> m_factoryFunctions;
};

template <typename ComponentInfoType>
void ActorComponentInfoFactory::RegisterFactoryFunction()
{
	RegisterFactoryFunction(ComponentInfoType::sk_typeName, &ComponentInfoType::LoadFromJSON);
}
}
