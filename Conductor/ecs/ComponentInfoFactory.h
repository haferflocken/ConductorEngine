#pragma once

#include <collection/VectorMap.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Asset { class AssetManager; }
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
	using FactoryFunction = Mem::UniquePtr<ComponentInfo>(*)(Asset::AssetManager&, const JSON::JSONObject&);

	ComponentInfoFactory();

	template <typename ComponentInfoType>
	void RegisterFactoryFunction();

	Mem::UniquePtr<ComponentInfo> MakeComponentInfo(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject) const;

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
