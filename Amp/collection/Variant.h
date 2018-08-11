#pragma once

#include <dev/Dev.h>
#include <util/VariadicUtil.h>

#include <functional>

namespace Collection
{
template <typename... Types>
class Variant final
{
public:
	Variant()
		: m_tagBytes{ 0 }
		, m_data()
	{}

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
	static constexpr size_t k_numTagBytes = Util::MaxAlignOf<Types...>();
	static constexpr size_t k_numDataBytes = Util::MaxAlignedSizeOf<Types...>();

	// The tag. Only the first byte of m_tagBytes is used for the tag;
	// the rest of the bytes are used to properly align m_data.
	uint8_t m_tagBytes[k_numTagBytes];
	uint8_t m_data[k_numDataBytes];
};

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

template <typename... Types>
template <typename... FnTypes>
void Variant<Types...>::Match(FnTypes&&... functions)
{
	if (m_tagBytes[0] == 0)
	{
		return;
	}

	std::function<void()> lambdas[]
	{
		[&]() { functions(reinterpret_cast<Types&>(m_data)); }...
	};

	lambdas[m_tagBytes[0]]();
}
}
