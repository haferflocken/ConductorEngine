#pragma once

#include <dev/Dev.h>
#include <util/VariadicUtil.h>

namespace Collection
{
template <typename... Types>
class Variant final
{
public:
	struct Tag final
	{
		size_t index;
	};

	static constexpr size_t sk_sizeInBytes = Util::MaxSizeOf<Types>();
	static constexpr size_t sk_numTags = (1 + sizeof...(Types));
	static constexpr Tag sk_tagInvalid{ 0 };

	Variant()
		: m_tag(sk_tagInvalid)
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
	
private:
	size_t m_tag;
	uint8_t m_data[sk_sizeInBytes];
};

template <typename... Types>
template <size_t Index>
Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get()
{
	Dev::FatalAssert(m_tag == Index, "Mismatch between current type and desired type in Variant.");
	return reinterpret_cast<Util::TypeAtIndex<Index, Types...>&>(m_data);
}

template <typename... Types>
template <size_t Index>
const Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get() const
{
	Dev::FatalAssert(m_tag == Index, "Mismatch between current type and desired type in Variant.");
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
}
