#pragma once

#include <traits/IsMemCopyAFullCopy.h>

#include <type_traits>

namespace Collection
{
template <typename FirstType, typename SecondType>
struct Pair
{
	FirstType first;
	SecondType second;

	// Copy-copy constructor.
	template <typename TypeA = FirstType, typename TypeB = SecondType>
	Pair(const std::enable_if_t<std::is_copy_constructible_v<TypeA>, TypeA>& f,
		const std::enable_if_t<std::is_copy_constructible_v<TypeB>, TypeB>& s)
		: first(f)
		, second(s)
	{}

	// Copy-move constructor.
	template <typename TypeA = FirstType, typename TypeB = SecondType>
	Pair(const std::enable_if_t<std::is_copy_constructible_v<TypeA>, TypeA>& f,
		std::enable_if_t<std::is_move_constructible_v<TypeB>, TypeB>&& s)
		: first(f)
		, second(std::move(s))
	{}

	// Move-copy constructor.
	template <typename TypeA = FirstType, typename TypeB = SecondType>
	Pair(std::enable_if_t<std::is_move_constructible_v<TypeA>, TypeA>&& f,
		const std::enable_if_t<std::is_copy_constructible_v<TypeB>, TypeB>& s)
		: first(std::move(f))
		, second(s)
	{}

	// Move-move constructor.
	template <typename TypeA = FirstType, typename TypeB = SecondType>
	Pair(std::enable_if_t<std::is_move_constructible_v<TypeA>, TypeA>&& f,
		std::enable_if_t<std::is_move_constructible_v<TypeB>, TypeB>&& s)
		: first(std::move(f))
		, second(std::move(s))
	{}

	Pair(const Pair&) = default;
	Pair(Pair&&) = default;

	Pair& operator=(const Pair&) = default;
	Pair& operator=(Pair&&) = default;
};
}

namespace Traits
{
/**
 * IsMemCopyAFullCopy specialization to indicate that a Pair can be memcpy'd if its elements can.
 */
template <typename FirstType, typename SecondType>
struct IsMemCopyAFullCopy<Collection::Pair<FirstType, SecondType>>
{
	static constexpr bool value = IsMemCopyAFullCopy<FirstType>::value && IsMemCopyAFullCopy<SecondType>::value;
};
}

