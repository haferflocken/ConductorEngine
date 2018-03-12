#pragma once

#include <type_traits>

namespace Collection { template <typename T> class Vector; }

namespace Util
{
/**
 * MaxSizeOf
 */
template <typename... Types>
constexpr size_t MaxSizeOf()
{
	size_t max = 0;
	const auto sizes = { sizeof(Types)... };
	for (const auto& size : sizes)
	{
		max = (size > max) ? size : max;
	}
	return max;
}

/**
 * TypeAtIndex
 */
template <size_t Index, typename... Types>
struct TypeAtIndexStruct;

template <typename FirstType, typename... RestTypes>
struct TypeAtIndexStruct<0, FirstType, RestTypes...>
{
	using Type = FirstType;
};

template <size_t Index, typename FirstType, typename... RestTypes>
struct TypeAtIndexStruct<Index, FirstType, RestTypes...>
{
	using Type = typename TypeAtIndexStruct<Index - 1, RestTypes...>::Type;
};

template <size_t Index, typename... Types>
using TypeAtIndex = typename TypeAtIndexStruct<Index, Types...>::Type;

/**
 * IndexOfType
 */
template <typename TargetType, typename... Types>
struct IndexOfTypeStruct;

template <typename TargetType, typename... Types>
struct IndexOfTypeStruct<TargetType, TargetType, Types...> : std::integral_constant<size_t, 0> {};

template <typename TargetType, typename FirstType, typename... RestTypes>
struct IndexOfTypeStruct<TargetType, FirstType, RestTypes...>
	: std::integral_constant<size_t, 1 + IndexOfTypeStruct<TargetType, RestTypes...>::value> {};

template <typename TargetType, typename... Types>
constexpr size_t IndexOfType = IndexOfTypeStruct<TargetType, Types...>::value;

/**
 * TypeList
 */
template <typename... Types>
struct TypeList
{
private:
	template <typename... OtherTypes>
	static constexpr TypeList<Types..., OtherTypes...> ConcatTypeFn(TypeList<OtherTypes...>) { return {}; }

public:
	static constexpr size_t Size = sizeof...(Types);

	template <size_t Index>
	using Get = TypeAtIndex<Index, Types...>;

	template <typename TargetType>
	static constexpr size_t IndexOfType() { return IndexOfType<TargetType, Types...>; }

	template <typename... OtherTypes>
	using AppendType = TypeList<Types..., OtherTypes...>;

	template <typename OtherListType>
	using ConcatType = decltype(ConcatTypeFn(OtherListType()));

	using ConstList = TypeList<std::add_const_t<Types>...>;
};

/**
 * MapArrayToTuple
 */
template <typename TupleType, typename ArrayValueType, size_t... Indices>
TupleType MapArrayToTuple(ArrayValueType* input, std::index_sequence<Indices...>)
{
	// Use dynamic_cast in debug builds to get nullptrs instead of garbage when a bad cast happens.
#ifdef _DEBUG
#define ELEMENT_CAST dynamic_cast
#else
#define ELEMENT_CAST static_cast
#endif
	// Cast each element of the array to its corresponding type in the tuple.
	return std::make_tuple(ELEMENT_CAST<std::tuple_element<Indices, TupleType>::type>(input[Indices])...);
#undef ELEMENT_CAST
}

template <typename TupleType, typename ArrayValueType>
TupleType MapArrayToTuple(ArrayValueType* input)
{
	constexpr size_t k_tupleSize = std::tuple_size_v<TupleType>;
	return MapArrayToTuple<TupleType, ArrayValueType>(input, std::make_index_sequence<k_tupleSize>());
}

/**
 * MapTupleToVector
 */
template <typename T, typename F, typename TupleType, size_t... Indices>
Collection::Vector<T> MapTupleToVector(F fn, TupleType& tuple, std::index_sequence<Indices...>)
{
	return Collection::Vector<T>({ fn(std::get<Indices>(tuple))... });
}

template <typename T, typename F, typename TupleType>
Collection::Vector<T> MapTupleToVector(F fn, TupleType& tuple)
{
	return MapTupleToVector<T, F, TupleType>(fn, tuple, std::make_index_sequence<std::tuple_size_v<TupleType>>());
}

/**
 * FillTupleWithValue
 */
template <typename TupleType, typename TupleValueType, size_t... Indices>
TupleType FillTupleWithValue(const TupleValueType& input, std::index_sequence<Indices...>)
{
	return std::make_tuple(static_cast<const std::tuple_element<Indices, TupleType>::type&>(input)...);
}

template <typename TupleType, typename TupleValueType>
TupleType FillTupleWithValue(const TupleValueType& input)
{
	return FillTupleWithValue<TupleType, TupleValueType>(input, std::make_index_sequence<std::tuple_size_v<TupleType>>());
}
}
