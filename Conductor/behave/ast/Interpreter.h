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
	// This may move data from parsedExpression into the return value.
	ExpressionCompileResult Compile(Parse::Expression& parsedExpression) const;

	// Evaluate an AST::Expression on the given entity.
	ExpressionResult EvaluateExpression(const Expression& expression, ECS::EntityManager& entityManager,
		const ECS::Entity& entity) const;

	// Binds a function so that it can be called with AST::Expressions as arguments. A bound function will always
	// receive a const ECS::Entity& as its first argument, followed by the arguments provided in the .behave file.
	// Because a behaviour tree may only access its entity's components, a bound function may only do the same.
	template <typename ReturnType, typename... ArgumentTypes>
	void BindFunction(const char* const functionName,
		ReturnType(*func)(const ECS::Entity&, ArgumentTypes...));

private:
	const ECS::ComponentReflector& m_componentReflector;

	struct OverloadInfo
	{
		BoundFunction m_boundFunction;
		const ExpressionResultTypeString* m_argumentTypeStrings;
		size_t m_numArguments;
	};
	// A map of function name hashes to possible overloads.
	Collection::VectorMap<Util::StringHash, Collection::Vector<OverloadInfo>> m_boundFunctionOverloads;
};
}

namespace Behave::AST
{
namespace Internal_Interpreter
{
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
			AMP_FATAL_ASSERT(val.GetTypeHash() == ComponentValueType::k_type.GetTypeHash(),
				"Mismatch between supported component type [%s] and argument [%s].",
				ComponentValueType::k_typeName, Util::ReverseHash(val.GetTypeHash()));

			ComponentValueType* const component = entityManager.FindComponent<ComponentValueType>(entity);
			AMP_FATAL_ASSERT(component != nullptr,
				"Could not find component [%s] when converting argument for bound function.",
				ComponentValueType::k_typeName);
			return static_cast<ArgType>(*component);
		}
		else
		{
			return static_cast<ArgType>(val);
		}
	}
};

template <typename Fn>
struct FunctionEvaluator;

template <typename ReturnType, typename... ArgumentTypes>
struct FunctionEvaluator<ReturnType(ArgumentTypes...)>
{
	static ExpressionResult Call(
		ECS::EntityManager& entityManager,
		const ECS::Entity& entity,
		ReturnType(*func)(const ECS::Entity&, ArgumentTypes...),
		ExpressionResult(&evaluatedArguments)[sizeof...(ArgumentTypes)])
	{
		ReturnType result = func(entity, ExpressionResultToArgument<ArgumentTypes>::Convert(entityManager, entity,
			evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ExpressionResultForArgument<ArgumentTypes>::type>())...);

		return ExpressionResult::Make<ReturnType>(std::move(result));
	}
};

template <typename... ArgumentTypes>
struct FunctionEvaluator<void(ArgumentTypes...)>
{
	static ExpressionResult Call(
		ECS::EntityManager& entityManager,
		const ECS::Entity& entity,
		void(*func)(const ECS::Entity&, ArgumentTypes...),
		ExpressionResult(&evaluatedArguments)[sizeof...(ArgumentTypes)])
	{
		func(entity, ExpressionResultToArgument<ArgumentTypes>::Convert(entityManager, entity,
			evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ExpressionResultForArgument<ArgumentTypes>::type>())...);

		return ExpressionResult::Make<None>();
	}
};

}

template <typename ReturnType, typename... ArgumentTypes>
inline void Interpreter::BindFunction(const char* const functionName,
	ReturnType(*func)(const ECS::Entity&, ArgumentTypes...))
{
	using namespace Internal_Interpreter;

	constexpr ExpressionResultTypeString k_returnType = ExpressionResultTypeString::Make<ReturnType>();

	struct BindingFunctions
	{
		static ExpressionResult Call(
			const Interpreter& interpreter,
			void* untypedFunc,
			const Collection::Vector<AST::Expression>& expressions,
			ECS::EntityManager& entityManager,
			const ECS::Entity& entity)
		{
			AMP_FATAL_ASSERT(sizeof...(ArgumentTypes) == expressions.Size(), "Expected %zu arguments, but got %u.",
				sizeof...(ArgumentTypes), expressions.Size());

			auto* func = static_cast<ReturnType(*)(const ECS::Entity&, ArgumentTypes...)>(untypedFunc);

			ExpressionResult evaluatedArguments[sizeof...(ArgumentTypes)];
			for (size_t i = 0; i < sizeof...(ArgumentTypes); ++i)
			{
				evaluatedArguments[i] = interpreter.EvaluateExpression(expressions[i], entityManager, entity);
			}

			return FunctionEvaluator<ReturnType(ArgumentTypes...)>::Call(entityManager, entity, func, evaluatedArguments);
		}
	};

	// Get the overloads for this function name.
	const Util::StringHash functionNameHash = Util::CalcHash(functionName);
	Collection::Vector<OverloadInfo>& overloads = m_boundFunctionOverloads[functionNameHash];

	// Because this function is templated on the argument types, a function-static array of argument type strings
	// can be made and then compared to using its pointer value.
	static const ExpressionResultTypeString k_argumentTypeStrings[]{ ExpressionResultTypeString::Make<ArgumentTypes>()... };
	
	AMP_FATAL_ASSERT(overloads.Find([](const OverloadInfo& overloadInfo)
		{
			return overloadInfo.m_argumentTypeStrings == k_argumentTypeStrings;
		}) == nullptr, "Cannot bind a function multiple times!");

	// Store the bound function.
	OverloadInfo& overload = overloads.Emplace();
	overload.m_boundFunction = BoundFunction(func, &BindingFunctions::Call, k_returnType);
	overload.m_argumentTypeStrings = k_argumentTypeStrings;
	overload.m_numArguments = sizeof...(ArgumentTypes);
}
}
