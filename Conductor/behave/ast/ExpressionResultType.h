#pragma once

#include <collection/Variant.h>
#include <ecs/ComponentType.h>

#include <type_traits>

namespace Behave::AST
{
// TODO(behave) move this elsewhere
struct TreeIdentifier
{
	Util::StringHash m_treeNameHash;
};

struct None {};

using ExpressionResult = Collection::Variant<None, bool, double, std::string, ECS::ComponentType, TreeIdentifier>;

struct ExpressionResultTypeString
{
	const char* m_typeString;

	template <typename T>
	static constexpr ExpressionResultTypeString Make()
	{
		return ExpressionResultTypeString{ ArgumentTypeString<std::remove_cv_t<std::remove_reference_t<T>>>::value };
	}

	constexpr ExpressionResultTypeString()
		: m_typeString(nullptr)
	{}

	explicit constexpr ExpressionResultTypeString(const char* typeString)
		: m_typeString(typeString)
	{}

	bool operator==(const ExpressionResultTypeString& rhs) const
	{
		return strcmp(m_typeString, rhs.m_typeString) == 0;
	}

	bool operator!=(const ExpressionResultTypeString& rhs) const
	{
		return !(*this == rhs);
	}

private:
	template <typename T>
	struct ArgumentTypeString;

	template <> struct ArgumentTypeString<void> { static constexpr const char* value = "NONE"; };
	template <> struct ArgumentTypeString<bool> { static constexpr const char* value = "BOOL"; };
	template <> struct ArgumentTypeString<double> { static constexpr const char* value = "DOUBLE"; };
	template <> struct ArgumentTypeString<std::string> { static constexpr const char* value = "STRING"; };
	template <> struct ArgumentTypeString<ECS::ComponentType> { static constexpr const char* value = "COMPONENTTYPE"; };
	template <> struct ArgumentTypeString<AST::TreeIdentifier> { static constexpr const char* value = "TREEIDENTIFIER"; };

	template <typename T>
	struct ArgumentTypeString
	{
		// Assume it is a component and extracts the type name from it.
		static constexpr const char* value = T::k_typeName;
	};

};
}
