#pragma once

#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourNode;
class BehaviourNodeFactory;

namespace Parse { struct ParsedTree; }

class BehaviourTree final
{
public:
	BehaviourTree();

	BehaviourTree(const BehaviourTree&) = delete;
	BehaviourTree& operator=(const BehaviourTree&) = delete;

	BehaviourTree(BehaviourTree&&) noexcept;
	void operator=(BehaviourTree&&) noexcept;

	~BehaviourTree();

	bool LoadFromParsedTree(const BehaviourNodeFactory& nodeFactory, Parse::ParsedTree& parsedTree);

	const BehaviourNode* GetRoot() const { return m_root.Get(); }

private:
	Mem::UniquePtr<BehaviourNode> m_root;
};
}
