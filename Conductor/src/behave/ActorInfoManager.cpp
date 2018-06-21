#include <behave/ActorInfoManager.h>

#include <behave/ActorComponentInfo.h>
#include <behave/ActorComponentInfoFactory.h>
#include <behave/ActorInfo.h>
#include <behave/BehaviourTreeManager.h>

#include <dev/Dev.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>

namespace Internal_ActorInfoManager
{
using namespace Behave;

const Util::StringHash k_componentsHash = Util::CalcHash("components");

Mem::UniquePtr<ActorInfo> MakeActorInfo(const ActorComponentInfoFactory& actorComponentInfoFactory,
	const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	const JSON::JSONArray* const componentsArray = jsonObject.FindArray(k_componentsHash);
	if (componentsArray == nullptr)
	{
		Dev::LogWarning("Failed to find component info array for actor info.");
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
		Mem::UniquePtr<ActorComponentInfo> componentInfo =
			actorComponentInfoFactory.MakeComponentInfo(behaviourTreeManager, valueObject);
		if (componentInfo == nullptr)
		{
			Dev::LogWarning("Failed to make an actor component info.");
			continue;
		}
		actorInfo->m_componentInfos.Add(std::move(componentInfo));
	}

	return actorInfo;
}
}

void Behave::ActorInfoManager::LoadActorInfosInDirectory(const File::Path& directory)
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

			Mem::UniquePtr<ActorInfo> actorInfo = Internal_ActorInfoManager::MakeActorInfo(
				m_actorComponentInfoFactory, m_behaviourTreeManager, jsonObject);
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

const Behave::ActorInfo* Behave::ActorInfoManager::FindActorInfo(const Util::StringHash actorInfoNameHash) const
{
	const auto itr = m_actorInfos.Find(actorInfoNameHash);
	return (itr != m_actorInfos.end()) ? &itr->second : nullptr;
}