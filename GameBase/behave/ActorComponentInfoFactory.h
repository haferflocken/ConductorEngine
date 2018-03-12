#pragma once

#include <mem/UniquePtr.h>
#include <util/StringHash.h>

#include <unordered_map>

namespace JSON { class JSONObject; }

namespace Behave
{
class ActorComponentInfo;

/**
 * Creates actor component info structs from JSON files.
 */
class ActorComponentInfoFactory
{
public:
	using FactoryFunction = Mem::UniquePtr<ActorComponentInfo>(*)(const JSON::JSONObject&);

	ActorComponentInfoFactory();

	template <typename ComponentInfoType>
	void RegisterFactoryFunction();

	Mem::UniquePtr<ActorComponentInfo> MakeComponentInfo(const JSON::JSONObject& jsonObject) const;

private:
	void RegisterFactoryFunction(const char* const componentTypeName, FactoryFunction fn);
	
	// Maps hashes of component type names to factory functions for those component types.
	std::unordered_map<Util::StringHash, FactoryFunction> m_factoryFunctions;
};

template <typename ComponentInfoType>
void ActorComponentInfoFactory::RegisterFactoryFunction()
{
	RegisterFactoryFunction(ComponentInfoType::sk_typeName, &ComponentInfoType::LoadFromJSON);
}
}
