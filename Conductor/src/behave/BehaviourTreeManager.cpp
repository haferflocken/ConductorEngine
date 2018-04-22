#include <behave/BehaviourTreeManager.h>
#include <behave/BehaviourTree.h>

#include <dev/Dev.h>
#include <file/JSONReader.h>
#include <json/JSONTypes.h>

Behave::BehaviourTreeManager::BehaviourTreeManager(const BehaviourNodeFactory& nodeFactory)
	: m_nodeFactory(nodeFactory)
	, m_trees()
{
}

Behave::BehaviourTreeManager::~BehaviourTreeManager()
{
}

void Behave::BehaviourTreeManager::LoadTreesInDirectory(const File::Path& directory)
{
	if (!File::IsDirectory(directory))
	{
		Dev::LogWarning("Cannot load behaviour trees in \"%s\" because it is not a directory.", directory.c_str());
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

			BehaviourTree tree;
			if (tree.LoadFromJSON(m_nodeFactory, jsonObject))
			{
				const File::Path& fileName = file.filename();
				m_trees[Util::CalcHash(fileName.string())] = std::move(tree);
			}
			else
			{
				Dev::LogWarning("Failed to load behaviour tree from \"%s\".", file.c_str());
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

const Behave::BehaviourTree* Behave::BehaviourTreeManager::FindTree(const Util::StringHash treeNameHash) const
{
	const auto itr = m_trees.Find(treeNameHash);
	return (itr != m_trees.end()) ? &itr->second : nullptr;
}
