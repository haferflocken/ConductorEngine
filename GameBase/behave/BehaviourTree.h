#pragma once

#include <mem/UniquePtr.h>

namespace JSON
{
class JSONObject;
}

namespace Behave
{
class BehaviourNode;
class BehaviourNodeFactory;

class BehaviourTree final
{
public:
	BehaviourTree();

	BehaviourTree(const BehaviourTree&) = delete;
	BehaviourTree& operator=(const BehaviourTree&) = delete;

	BehaviourTree(BehaviourTree&&);
	void operator=(BehaviourTree&&);

	~BehaviourTree();

	bool LoadFromJSON(const BehaviourNodeFactory& nodeFactory, const JSON::JSONObject& jsonObject);

	const BehaviourNode* GetRoot() const { return m_root.Get(); }

private:
	Mem::UniquePtr<BehaviourNode> m_root;
};
}
