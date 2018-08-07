#include <ecs/EntityInfoManager.h>

#include <ecs/ComponentInfo.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/EntityInfo.h>

#include <dev/Dev.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>

namespace Internal_EntityInfoManager
{
using namespace ECS;

const Util::StringHash k_componentsHash = Util::CalcHash("components");

Mem::UniquePtr<EntityInfo> MakeEntityInfo(const ComponentInfoFactory& componentInfoFactory,
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject,
	const Util::StringHash nameHash)
{
	const JSON::JSONArray* const componentsArray = jsonObject.FindArray(k_componentsHash);
	if (componentsArray == nullptr)
	{
		Dev::LogWarning("Failed to find component info array for entity info.");
		return nullptr;
	}

	auto entityInfo = Mem::MakeUnique<EntityInfo>();
	entityInfo->m_nameHash = nameHash;

	for (const auto& value : *componentsArray)
	{
		if (value->GetType() != JSON::ValueType::Object)
		{
			Dev::LogWarning("Encountered a non-object element in an component info array.");
			continue;
		}
		const JSON::JSONObject& valueObject = static_cast<const JSON::JSONObject&>(*value);
		Mem::UniquePtr<ComponentInfo> componentInfo =
			componentInfoFactory.MakeComponentInfo(behaviourTreeManager, valueObject);
		if (componentInfo == nullptr)
		{
			Dev::LogWarning("Failed to make an component info.");
			continue;
		}
		entityInfo->m_componentInfos.Add(std::move(componentInfo));
	}

	return entityInfo;
}
}

void ECS::EntityInfoManager::LoadEntityInfosInDirectory(const File::Path& directory)
{
	if (!File::IsDirectory(directory))
	{
		Dev::LogWarning("Cannot load entity infos in \"%s\" because it is not a directory.", directory.c_str());
		return;
	}

	File::ForEachFileInDirectory(directory, [this](const File::Path& file) -> bool
	{
		Mem::UniquePtr<JSON::JSONValue> jsonValue = File::ReadJSONFile(file);
		
		switch (jsonValue->GetType())
		{
		case JSON::ValueType::Object:
		{
			const JSON::JSONObject& jsonObject = *static_cast<const JSON::JSONObject*>(jsonValue.Get());

			const File::Path& fileName = file.filename();
			const Util::StringHash fileNameHash = Util::CalcHash(fileName.string());

			Mem::UniquePtr<EntityInfo> entityInfo = Internal_EntityInfoManager::MakeEntityInfo(
				m_componentInfoFactory, m_behaviourTreeManager, jsonObject, fileNameHash);
			if (entityInfo != nullptr)
			{
				m_entityInfos[fileNameHash] = std::move(*entityInfo);
			}
			else
			{
				Dev::LogWarning("Failed to load entity info from \"%s\".", file.c_str());
			}
			break;
		}
		default:
		{
			Dev::LogWarning("Failed to find a JSON object at the root of \"%s\".", file.c_str());
			break;
		}
		}

		// Return true to keep iterating over the files.
		return true;
	});
}

const ECS::EntityInfo* ECS::EntityInfoManager::FindEntityInfo(const Util::StringHash entityInfoNameHash) const
{
	const auto itr = m_entityInfos.Find(entityInfoNameHash);
	return (itr != m_entityInfos.end()) ? &itr->second : nullptr;
}
