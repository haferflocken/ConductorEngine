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
class Component;
class ComponentReflector;
class Entity;
class EntityManager;
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
	ExpressionResult EvaluateExpression(const Expression& expression, ECS::EntityManager& entityManager,
		const ECS::Entity& entity) const;

	// Binds a function so that it can be called with AST::Expressions as arguments. A bound function will always
	// receive a const ECS::Entity& as its first argument, followed by the arguments provided in the .behave file.
	// Because a behaviour tree may only access its entity's components, a bound function may only do the same.
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

template <typename T>
struct ExpressionResultForArgument;

template <> struct ExpressionResultForArgument<bool> { using type = bool; };
template <> struct ExpressionResultForArgument<const bool> { using type = bool; };
template <> struct ExpressionResultForArgument<bool&> { using type = bool; };
template <> struct ExpressionResultForArgument<const bool&> { using type = bool; };
template <> struct ExpressionResultForArgument<double> { using type = double; };
template <> struct ExpressionResultForArgument<const double> { using type = double; };
template <> struct ExpressionResultForArgument<double&> { using type = double; };
template <> struct ExpressionResultForArgument<const double&> { using type = double; };
template <> struct ExpressionResultForArgument<std::string> { using type = std::string; };
template <> struct ExpressionResultForArgument<const std::string> { using type = std::string; };
template <> struct ExpressionResultForArgument<std::string&> { using type = std::string; };
template <> struct ExpressionResultForArgument<const std::string&> { using type = std::string; };
template <> struct ExpressionResultForArgument<ECS::ComponentType> { using type = ECS::ComponentType; };
template <> struct ExpressionResultForArgument<const ECS::ComponentType> { using type = ECS::ComponentType; };
template <> struct ExpressionResultForArgument<ECS::ComponentType&> { using type = ECS::ComponentType; };
template <> struct ExpressionResultForArgument<const ECS::ComponentType&> { using type = ECS::ComponentType; };
template <> struct ExpressionResultForArgument<TreeIdentifier> { using type = TreeIdentifier; };
template <> struct ExpressionResultForArgument<const TreeIdentifier> { using type = TreeIdentifier; };
template <> struct ExpressionResultForArgument<TreeIdentifier&> { using type = TreeIdentifier; };
template <> struct ExpressionResultForArgument<const TreeIdentifier&> { using type = TreeIdentifier; };

template <typename T>
inline constexpr bool IsComponentReference = std::is_reference_v<T> && std::is_convertible_v<T, const ECS::Component&>;

template <typename T>
struct ExpressionResultForArgument
{
	using type = std::enable_if_t<IsComponentReference<T>, ECS::ComponentType>;
};

template <typename ArgType, typename ExpResultType = ExpressionResultForArgument<ArgType>::type>
struct ExpressionResultToArgument
{
	static ArgType Convert(ECS::EntityManager& entityManager, const ECS::Entity& entity, ExpResultType& val)
	{
		if constexpr(IsComponentReference<ArgType>)
		{
			using ComponentValueType = std::remove_reference_t<ArgType>;
			Dev::FatalAssert(val.GetTypeHash() == ComponentValueType::Info::sk_typeHash,
				"Mismatch between supported component type [%s] and argument [%s].",
				ComponentValueType::Info::sk_typeName, Util::ReverseHash(val.GetTypeHash()));

			ComponentValueType* const component = entityManager.FindComponent<ComponentValueType>(entity);
			Dev::FatalAssert(component != nullptr,
				"Could not find component [%s] when converting argument for bound function.",
				ComponentValueType::Info::sk_typeName);
			return static_cast<ArgType>(*component);
		}
		else
		{
			return static_cast<ArgType>(val);
		}
	}
};
}

template <typename ReturnType, typename... ArgumentTypes>
inline void Interpreter::BindFunction(const Util::StringHash functionNameHash,
	ReturnType(*func)(const ECS::Entity&, ArgumentTypes...))
{
	using namespace Internal_Interpreter;

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
			ECS::EntityManager& entityManager,
			const ECS::Entity& entity)
		{
			Dev::FatalAssert(sizeof...(ArgumentTypes) == expressions.Size(), "Expected %zu arguments, but got %u.",
				sizeof...(ArgumentTypes), expressions.Size());

			auto* func = static_cast<ReturnType(*)(const ECS::Entity&, ArgumentTypes...)>(untypedFunc);

			ExpressionResult evaluatedArguments[sizeof...(ArgumentTypes)];
			for (size_t i = 0; i < sizeof...(ArgumentTypes); ++i)
			{
				evaluatedArguments[i] = interpreter.EvaluateExpression(expressions[i], entityManager, entity);
			}

			ReturnType result = func(entity, ExpressionResultToArgument<ArgumentTypes>::Convert(entityManager, entity,
				evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ExpressionResultForArgument<ArgumentTypes>::type>())...);

			return ExpressionResult::Make<ReturnType>(std::move(result));
		}
	};

	Dev::FatalAssert(m_boundFunctions.Find(functionNameHash) == m_boundFunctions.end()
		&& m_boundFunctionArgumentTypes.Find(functionNameHash) == m_boundFunctionArgumentTypes.end(),
		"Cannot bind a function multiple times!");

	m_boundFunctions[functionNameHash] = BoundFunction(func, &BindingFunctions::Call, k_returnType);
	auto& argumentTypes = m_boundFunctionArgumentTypes[functionNameHash];

	for (const auto& t : { ExpressionResultTypeFor<ExpressionResultForArgument<ArgumentTypes>::type>::value... })
	{
		argumentTypes.Add(t);
	}
}
}
