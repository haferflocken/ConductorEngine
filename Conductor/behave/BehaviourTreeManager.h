#pragma once

#include <collection/VectorMap.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace Behave
{
class BehaviourNodeFactory;
class BehaviourTree;

/**
 * Loads and owns behaviour trees. A behaviour tree manager should be instantiated for each "class" of trees
 * being loaded, as each has its own node factory and can therefore support different types of trees.
 */
class BehaviourTreeManager final
{
public:
	BehaviourTreeManager(const BehaviourNodeFactory& nodeFactory);
	~BehaviourTreeManager();

	void LoadTreesInDirectory(const File::Path& directory);

	const BehaviourTree* FindTree(const Util::StringHash treeNameHash) const;

private:
	const BehaviourNodeFactory& m_nodeFactory;
	Collection::VectorMap<Util::StringHash, BehaviourTree> m_trees;
};
}
