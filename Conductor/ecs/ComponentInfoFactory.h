#pragma once

#include <collection/VectorMap.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Behave { class BehaviourTreeManager; }
namespace JSON { class JSONObject; }

namespace ECS
{
class ComponentInfo;

/**
 * Creates component info structs from JSON files.
 */
class ComponentInfoFactory
{
public:
	using FactoryFunction = Mem::UniquePtr<ComponentInfo>(*)(const Behave::BehaviourTreeManager&, const JSON::JSONObject&);

	ComponentInfoFactory();

	template <typename ComponentInfoType>
	void RegisterFactoryFunction();

	Mem::UniquePtr<ComponentInfo> MakeComponentInfo(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject) const;

private:
	void RegisterFactoryFunction(const char* const componentTypeName, FactoryFunction fn);
	
	// Maps hashes of component type names to factory functions for those component types.
	Collection::VectorMap<Util::StringHash, FactoryFunction> m_factoryFunctions;
};

template <typename ComponentInfoType>
void ComponentInfoFactory::RegisterFactoryFunction()
{
	RegisterFactoryFunction(ComponentInfoType::sk_typeName, &ComponentInfoType::LoadFromJSON);
}
}
