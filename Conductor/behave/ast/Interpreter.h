#pragma once

#include <behave/ast/ASTTypes.h>
#include <behave/ast/BoundFunction.h>
#include <behave/ast/ExpressionResultType.h>

#include <collection/Variant.h>
#include <collection/VectorMap.h>
#include <util/StringHash.h>

namespace Behave::Parse { struct Expression; }
namespace ECS
{
class ComponentReflector;
class Entity;
}

namespace Behave::AST
{
struct TypeCheckFailure
{
	explicit TypeCheckFailure(const char* message)
		: m_message(message)
	{}

	explicit TypeCheckFailure(std::string&& message)
		: m_message(std::move(message))
	{}

	std::string m_message;
};

using ExpressionCompileResult = Collection::Variant<Expression, TypeCheckFailure>;

/**
 * An interpreter that evaluates AST expressions.
 */
class Interpreter
{
public:
	explicit Interpreter(const ECS::ComponentReflector& componentReflector);
	~Interpreter();

	// Compile a parsed expression into an executable expression.
	ExpressionCompileResult Compile(const Parse::Expression& parsedExpression) const;

	// Evaluate an AST::Expression on the given entity.
	ExpressionResult EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const;

	// Binds a function so that it can be called with AST::Expressions as arguments.
	template <typename ReturnType, typename... ArgumentTypes>
	void BindFunction(const Util::StringHash functionNameHash,
		ReturnType(*func)(const ECS::Entity&, ArgumentTypes...));

private:
	const ECS::ComponentReflector& m_componentReflector;

	Collection::VectorMap<Util::StringHash, BoundFunction> m_boundFunctions;
	Collection::VectorMap<Util::StringHash, Collection::Vector<ExpressionResultTypes>> m_boundFunctionArgumentTypes;
};
}

namespace Behave::AST
{
namespace Internal_Interpreter
{
template <typename T>
struct ExpressionResultTypeFor;

template <> struct ExpressionResultTypeFor<bool>
{
	static constexpr ExpressionResultTypes value = ExpressionResultTypes::Boolean;
};

template <> struct ExpressionResultTypeFor<double>
{
	static constexpr ExpressionResultTypes value = ExpressionResultTypes::Number;
};

template <> struct ExpressionResultTypeFor<std::string>
{
	static constexpr ExpressionResultTypes value = ExpressionResultTypes::String;
};

template <> struct ExpressionResultTypeFor<ECS::ComponentType>
{
	static constexpr ExpressionResultTypes value = ExpressionResultTypes::ComponentType;
};

template <> struct ExpressionResultTypeFor<TreeIdentifier>
{
	static constexpr ExpressionResultTypes value = ExpressionResultTypes::TreeIdentifier;
};
}

template <typename ReturnType, typename... ArgumentTypes>
inline void Interpreter::BindFunction(const Util::StringHash functionNameHash,
	ReturnType(*func)(const ECS::Entity&, ArgumentTypes...))
{
	constexpr ExpressionResultTypes k_returnType =
		std::is_same_v<ReturnType, bool> ? ExpressionResultTypes::Boolean
		: std::is_same_v<ReturnType, double> ? ExpressionResultTypes::Number
		: std::is_same_v<ReturnType, std::string> ? ExpressionResultTypes::String
		: std::is_same_v<ReturnType, ECS::ComponentType> ? ExpressionResultTypes::ComponentType
		: ExpressionResultTypes::TreeIdentifier;

	static_assert(k_returnType != ExpressionResultTypes::TreeIdentifier || std::is_same_v<ReturnType, TreeIdentifier>,
		"Cannot bind a function that does not return bool, double, ECS::ComponentType, or TreeIdentifier.");

	struct BindingFunctions
	{
		static ExpressionResult Call(
			const Interpreter& interpreter,
			void* untypedFunc,
			const Collection::Vector<AST::Expression>& expressions,
			const ECS::Entity& entity)
		{
			Dev::FatalAssert(sizeof...(ArgumentTypes) == expressions.Size(), "Expected %zu arguments, but got %u.",
				sizeof...(ArgumentTypes), expressions.Size());

			auto* func = static_cast<ReturnType(*)(const ECS::Entity&, ArgumentTypes...)>(untypedFunc);

			ExpressionResult evaluatedArguments[sizeof...(ArgumentTypes)];
			for (size_t i = 0; i < sizeof...(ArgumentTypes); ++i)
			{
				evaluatedArguments[i] = interpreter.EvaluateExpression(expressions[i], entity);
			}

			ReturnType result = func(entity,
				evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ArgumentTypes>()...);

			return ExpressionResult::Make<ReturnType>(std::move(result));
		}
	};

	Dev::FatalAssert(m_boundFunctions.Find(functionNameHash) == m_boundFunctions.end()
		&& m_boundFunctionArgumentTypes.Find(functionNameHash) == m_boundFunctionArgumentTypes.end(),
		"Cannot bind a function multiple times!");

	m_boundFunctions[functionNameHash] = BoundFunction(func, &BindingFunctions::Call, k_returnType);
	auto& argumentTypes = m_boundFunctionArgumentTypes[functionNameHash];

	for (const auto& t : { Internal_Interpreter::ExpressionResultTypeFor<ArgumentTypes>::value... })
	{
		argumentTypes.Add(t);
	}
}
}
