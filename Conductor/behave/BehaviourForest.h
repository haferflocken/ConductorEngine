#pragma once

#include <behave/BehaviourTree.h>
#include <collection/VectorMap.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace Behave
{
class BehaviourNodeFactory;

/**
 * A collection of behaviour trees that can be loaded through the asset system.
 */
class BehaviourForest final
{
public:
	static bool TryLoad(const BehaviourNodeFactory& nodeFactory,
		const File::Path& filePath,
		BehaviourForest* destination);

	BehaviourForest(Collection::Vector<std::string>&& imports,
		Collection::VectorMap<Util::StringHash, BehaviourTree>&& trees);
	~BehaviourForest();

	const Collection::Vector<std::string>& GetImports() const;
	const BehaviourTree* FindTree(const Util::StringHash treeNameHash) const;

private:
	Collection::Vector<std::string> m_imports;
	Collection::VectorMap<Util::StringHash, BehaviourTree> m_trees;
};
}

// Inline implementations.
namespace Behave
{
inline const Collection::Vector<std::string>& BehaviourForest::GetImports() const
{
	return m_imports;
}
}
