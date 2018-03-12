#include <behave/ActorInfoManager.h>

#include <behave/ActorComponentInfo.h>
#include <behave/ActorComponentInfoFactory.h>
#include <behave/ActorInfo.h>
#include <behave/BehaviourTreeManager.h>

#include <dev/Dev.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>

using namespace Behave;

namespace
{
const Util::StringHash k_componentsHash = Util::CalcHash("components");
const Util::StringHash k_behaviourTreesArray = Util::CalcHash("behaviour_trees");

Mem::UniquePtr<ActorInfo> MakeActorInfo(const ActorComponentInfoFactory& actorComponentInfoFactory,
	const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	const JSON::JSONArray* const componentsArray = jsonObject.FindArray(k_componentsHash);
	if (componentsArray == nullptr)
	{
		Dev::LogWarning("Failed to find component info array for actor info.");
		return nullptr;
	}

	const JSON::JSONArray* const behaviourTreesArray = jsonObject.FindArray(k_behaviourTreesArray);
	if (behaviourTreesArray == nullptr)
	{
		Dev::LogWarning("Failed to find behaviour tree array for actor info.");
		return nullptr;
	}

	auto actorInfo = Mem::MakeUnique<ActorInfo>();

	for (const auto& value : *componentsArray)
	{
		if (value->GetType() != JSON::ValueType::Object)
		{
			Dev::LogWarning("Encountered a non-object element in an actor component info array.");
			continue;
		}
		const JSON::JSONObject& valueObject = static_cast<const JSON::JSONObject&>(*value);
		Mem::UniquePtr<ActorComponentInfo> componentInfo = actorComponentInfoFactory.MakeComponentInfo(valueObject);
		if (componentInfo == nullptr)
		{
			Dev::LogWarning("Failed to make an actor component info.");
			continue;
		}
		actorInfo->m_componentInfos.Add(std::move(componentInfo));
	}

	for (const auto& value : *behaviourTreesArray)
	{
		if (value->GetType() != JSON::ValueType::String)
		{
			Dev::LogWarning("Encountered a non-string element in an actor info behaviour tree array.");
			continue;
		}
		const JSON::JSONString& valueString = static_cast<const JSON::JSONString&>(*value);
		const BehaviourTree* const behaviourTree = behaviourTreeManager.FindTree(valueString.m_hash);
		if (behaviourTree == nullptr)
		{
			Dev::LogWarning("Failed to find behaviour tree \"%s\".", valueString.m_string.c_str());
			continue;
		}
		actorInfo->m_behaviourTrees.Add(behaviourTree);
	}

	return actorInfo;
}
}

void ActorInfoManager::LoadActorInfosInDirectory(const File::Path& directory)
{
	if (!File::IsDirectory(directory))
	{
		Dev::LogWarning("Cannot load actor infos in \"%s\" because it is not a directory.", directory.c_str());
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

			Mem::UniquePtr<ActorInfo> actorInfo =
				MakeActorInfo(m_actorComponentInfoFactory, m_behaviourTreeManager, jsonObject);
			if (actorInfo != nullptr)
			{
				const File::Path& fileName = file.filename();
				m_actorInfos[Util::CalcHash(fileName.string())] = std::move(*actorInfo);
			}
			else
			{
				Dev::LogWarning("Failed to load actor info from \"%s\".", file.c_str());
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

const ActorInfo* ActorInfoManager::FindActorInfo(const Util::StringHash actorInfoNameHash) const
{
	const auto itr = m_actorInfos.find(actorInfoNameHash);
	return (itr != m_actorInfos.end()) ? &itr->second : nullptr;
}
