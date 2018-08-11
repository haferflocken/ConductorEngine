#pragma once

#include <dev/Dev.h>
#include <util/VariadicUtil.h>

#include <functional>

namespace Collection
{
template <typename... Types>
class alignas(Types...) Variant final
{
public:
	Variant()
		: m_tagBytes{ k_invalidTag }
		, m_data()
	{}

	template <typename T, size_t Index = Util::IndexOfType<T, Types...>>
	Variant(const T& v)
		: m_tagBytes{ static_cast<uint8_t>(Index) }
	{
		new (m_data) T(v);
	}

	template <typename T, size_t Index = Util::IndexOfType<T, Types...>>
	Variant(T&& v)
		: m_tagBytes{ static_cast<uint8_t>(Index) }
	{
		new (m_data) T(std::move(v));
	}

	Variant(const Variant& other);
	Variant& operator=(const Variant& rhs);

	Variant(Variant&& other);
	Variant& operator=(Variant&& rhs);

	template <size_t Index>
	Util::TypeAtIndex<Index, Types...>& Get();

	template <size_t Index>
	const Util::TypeAtIndex<Index, Types...>& Get() const;

	template <typename Type>
	Type& Get();

	template <typename Type>
	const Type& Get() const;

	template <typename... FnTypes>
	void Match(FnTypes&&... functions);

private:
	static constexpr uint8_t k_invalidTag = UINT8_MAX;
	static constexpr size_t k_numTagBytes = Util::MaxAlignOf<Types...>();
	static constexpr size_t k_numDataBytes = Util::MaxAlignedSizeOf<Types...>();

	// The tag. Only the first byte of m_tagBytes is used for the tag;
	// the rest of the bytes are used to properly align m_data.
	uint8_t m_tagBytes[k_numTagBytes];
	uint8_t m_data[k_numDataBytes];
};

template <typename... Types>
Variant<Types...>::Variant(const Variant& other)
	: m_tagBytes{ other.m_tagBytes[0] }
{
	// TODO(variant)
}

template <typename... Types>
Variant<Types...>& Variant<Types...>::operator=(const Variant& rhs)
{
	// TODO(variant)
	return *this;
}

template <typename... Types>
Variant<Types...>::Variant(Variant&& other)
	: m_tagBytes{ other.m_tagBytes[0] }
{
	// TODO(variant)
}

template <typename... Types>
Variant<Types...>& Variant<Types...>::operator=(Variant&& rhs)
{
	// TODO(variant)
	return *this;
}

template <typename... Types>
template <size_t Index>
Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get()
{
	Dev::FatalAssert(m_tagBytes[0] == Index, "Mismatch between current type and desired type in Variant.");
	return reinterpret_cast<Util::TypeAtIndex<Index, Types...>&>(m_data);
}

template <typename... Types>
template <size_t Index>
const Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get() const
{
	Dev::FatalAssert(m_tagBytes[0] == Index, "Mismatch between current type and desired type in Variant.");
	return reinterpret_cast<const Util::TypeAtIndex<Index, Types...>&>(m_data);
}

template <typename... Types>
template <typename Type>
Type& Variant<Types...>::Get()
{
	return Get<Util::IndexOfType<Type, Types...>>();
}

template <typename... Types>
template <typename Type>
const Type& Variant<Types...>::Get() const
{
	return Get<Util::IndexOfType<Type, Types...>>();
}

namespace Internal_Variant
{
template <typename FnTuple, size_t... Indices>
void CallIndexInTuple(FnTuple& t, size_t index, std::index_sequence<Indices...>)
{
	Util::Or(((index == Indices) ? (std::get<Indices>(t)(), true) : false)...);
}
}

template <typename... Types>
template <typename... FnTypes>
void Variant<Types...>::Match(FnTypes&&... functions)
{
	static_assert(sizeof...(Types) == sizeof...(FnTypes), "There must be exactly one function per type.");
	if (m_tagBytes[0] == k_invalidTag)
	{
		return;
	}

	auto lambdas = std::make_tuple(
		[&]() { functions(reinterpret_cast<Types&>(m_data)); }...
	);

	auto indexSequence = std::index_sequence_for<Types...>();
	Internal_Variant::CallIndexInTuple(lambdas, m_tagBytes[0], indexSequence);
}
}
