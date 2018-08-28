#pragma once

#include <behave/ast/ExpressionResultType.h>
#include <collection/VectorMap.h>
#include <util/StringHash.h>

namespace Behave
{
/**
 * A blackboard is a dictionary for data of several types, and is used to allow behaviour nodes to communicate.
 */
class Blackboard
{
public:
	Blackboard() = default;

	void GetWithDefault(const Util::StringHash& key, const AST::ExpressionResult& defaultValue,
		AST::ExpressionResult& out) const;
	
	bool TryRemove(const Util::StringHash& key);

	void Set(const Util::StringHash& key, const AST::ExpressionResult& value);

private:
	// A lookup of keys to values in the blackboard.
	Collection::VectorMap<Util::StringHash, AST::ExpressionResult> m_map;
};
}
