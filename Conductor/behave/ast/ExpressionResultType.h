#pragma once

#include <collection/Variant.h>
#include <ecs/ComponentType.h>

namespace Behave::AST
{
// TODO(behave) move this elsewhere
struct TreeIdentifier
{
	Util::StringHash m_treeNameHash;
};

using ExpressionResult = Collection::Variant<bool, double, std::string, ECS::ComponentType, TreeIdentifier>;

enum class ExpressionResultTypes : uint8_t
{
	Boolean,
	Number,
	String,
	ComponentType,
	TreeIdentifier
};
}
