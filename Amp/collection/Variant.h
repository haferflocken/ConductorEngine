#pragma once

#include <dev/Dev.h>
#include <util/VariadicUtil.h>

#include <functional>

namespace Collection
{
template <typename... Types>
class alignas(Types...) Variant
{
public:
	Variant();
	~Variant();

	template <typename T, typename... Args>
	static Variant Make(Args&&... args);

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

	template <typename Type>
	bool Is() const;

	bool IsAny() const;

	template <typename... FnTypes>
	void Match(FnTypes&&... functions);

	template <typename... FnTypes>
	void Match(FnTypes&&... functions) const;

private:
	static constexpr uint8_t k_invalidTag = UINT8_MAX;
	static constexpr size_t k_numTagBytes = Util::MaxAlignOf<Types...>();
	static constexpr size_t k_numDataBytes = Util::MaxAlignedSizeOf<Types...>();

	template <typename T>
	static void Destroy(T& data) { data.~T(); }

	// The tag. Only the first byte of m_tagBytes is used for the tag;
	// the rest of the bytes are used to properly align m_data.
	uint8_t m_tagBytes[k_numTagBytes];
	uint8_t m_data[k_numDataBytes];
};

template <typename... Types>
inline Variant<Types...>::Variant()
	: m_tagBytes{ k_invalidTag }
	, m_data()
{}

template <typename... Types>
inline Variant<Types...>::~Variant()
{
	// Parameter pack expansions don't work for destructor calls,
	// so a templated function to call the destructor is used instead.
	Match([](Types& data) { Destroy<Types>(data); }...);
	m_tagBytes[0] = k_invalidTag;
}

template <typename... Types>
inline Variant<Types...>::Variant(const Variant& other)
	: m_tagBytes{ other.m_tagBytes[0] }
{
	Match([&](Types& data) { new (&data) Types(reinterpret_cast<const Types&>(other.m_data)); }...);
}

template <typename... Types>
template <typename T, typename... Args>
inline Variant<Types...> Variant<Types...>::Make(Args&&... args)
{
	constexpr size_t k_index = Util::IndexOfType<T, Types...>;

	Variant out;
	out.m_tagBytes[0] = k_index;
	new (&out.m_data) T(std::forward<Args>(args)...);

	return out;
}

template <typename... Types>
inline Variant<Types...>& Variant<Types...>::operator=(const Variant& rhs)
{
	Match([](Types& data) { Destroy<Types>(data); }...);

	m_tagBytes[0] = rhs.m_tagBytes[0];
	Match([&](Types& data) { new (&data) Types(reinterpret_cast<const Types&>(rhs.m_data)); }...);

	return *this;
}

template <typename... Types>
inline Variant<Types...>::Variant(Variant&& other)
	: m_tagBytes{ other.m_tagBytes[0] }
{
	Match([&](Types& data) { new (&data) Types(std::move(reinterpret_cast<Types&>(other.m_data))); }...);
}

template <typename... Types>
inline Variant<Types...>& Variant<Types...>::operator=(Variant&& rhs)
{
	Match([](Types& data) { Destroy<Types>(data); }...);

	m_tagBytes[0] = rhs.m_tagBytes[0];
	Match([&](Types& data) { new (&data) Types(std::move(reinterpret_cast<Types&>(rhs.m_data))); }...);

	return *this;
}

template <typename... Types>
template <size_t Index>
inline Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get()
{
	Dev::FatalAssert(m_tagBytes[0] == Index, "Mismatch between current type and desired type in Variant.");
	return reinterpret_cast<Util::TypeAtIndex<Index, Types...>&>(m_data);
}

template <typename... Types>
template <size_t Index>
inline const Util::TypeAtIndex<Index, Types...>& Variant<Types...>::Get() const
{
	Dev::FatalAssert(m_tagBytes[0] == Index, "Mismatch between current type and desired type in Variant.");
	return reinterpret_cast<const Util::TypeAtIndex<Index, Types...>&>(m_data);
}

template <typename... Types>
template <typename Type>
inline Type& Variant<Types...>::Get()
{
	return Get<Util::IndexOfType<Type, Types...>>();
}

template <typename... Types>
template <typename Type>
inline const Type& Variant<Types...>::Get() const
{
	return Get<Util::IndexOfType<Type, Types...>>();
}

template <typename... Types>
template <typename Type>
inline bool Variant<Types...>::Is() const
{
	return m_tagBytes[0] == Util::IndexOfType<Type, Types...>;
}

template <typename... Types>
inline bool Variant<Types...>::IsAny() const
{
	return m_tagBytes[0] != k_invalidTag;
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
inline void Variant<Types...>::Match(FnTypes&&... functions)
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

template <typename... Types>
template <typename... FnTypes>
inline void Variant<Types...>::Match(FnTypes&&... functions) const
{
	static_assert(sizeof...(Types) == sizeof...(FnTypes), "There must be exactly one function per type.");
	if (m_tagBytes[0] == k_invalidTag)
	{
		return;
	}

	auto lambdas = std::make_tuple(
		[&]() { functions(reinterpret_cast<const Types&>(m_data)); }...
	);

	auto indexSequence = std::index_sequence_for<Types...>();
	Internal_Variant::CallIndexInTuple(lambdas, m_tagBytes[0], indexSequence);
}
}
