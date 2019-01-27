#pragma once

#include <collection/Vector.h>

#include <cstdint>
#include <initializer_list>
#include <typeinfo>

namespace Mem
{
struct InspectorInfo
{
	static void Register(const InspectorInfo& info);
	static const InspectorInfo* Find(size_t typeHash);

	struct MemberInfo
	{
		const char* m_name;
		size_t m_typeHash;
		size_t m_offset;
	};

	InspectorInfo(const std::type_info& typeInfo);

	InspectorInfo(const std::type_info& typeInfo,
		std::initializer_list<size_t> templateParameterTypeHashes);

	InspectorInfo(const std::type_info& typeInfo,
		std::initializer_list<const char*> memberNames,
		std::initializer_list<size_t> memberTypeHashes,
		std::initializer_list<size_t> memberOffsets);

	// m_typeName and m_typeHash are always defined for inspectable types.
	const char* m_typeName;
	size_t m_typeHash;
	// m_templateTypeHash and m_templateParameterTypeHashes are only defined for templated inspectable types.
	size_t m_templateTypeHash;
	Collection::Vector<size_t> m_templateParameterTypeHashes;
	// m_memberInfo is only defined for struct types.
	Collection::Vector<MemberInfo> m_memberInfo;
};

template <typename T>
struct InspectorInfo_Helper
{
	static InspectorInfo Info()
	{
		return InspectorInfo(typeid(T));
	}
};

template <template<typename...> class T, typename... Args>
struct InspectorInfo_Helper<T<Args...>>
{
	using Type = T<Args...>;

	static InspectorInfo Info()
	{
		return InspectorInfo(typeid(Type), { (InspectorInfo_Helper<Args>::Info().m_typeHash)... });
	}
};
}

#define EMPTY()
#define DEFER(A) A EMPTY()

#define STRINGIFY_ARGS_0()
#define STRINGIFY_ARGS_1(A0) #A0
#define STRINGIFY_ARGS_2(A0, A1) #A0 , #A1
#define STRINGIFY_ARGS_3(A0, A1, A2) #A0 , #A1 , #A2
#define STRINGIFY_ARGS_4(A0, A1, A2, A3) #A0 , #A1 , #A2 , #A3
#define STRINGIFY_ARGS_5(A0, A1, A2, A3, A4) #A0 , #A1 , #A2 , #A3 , #A4
#define STRINGIFY_ARGS_6(A0, A1, A2, A3, A4, A5) #A0 , #A1 , #A2 , #A3 , #A4 , #A5

#define TYPEHASH_OF_ARGS_0(...)
#define TYPEHASH_OF_ARGS_1(TYPE, A0) Mem::InspectorInfo_Helper<decltype(TYPE :: A0)>::Info().m_typeHash
#define TYPEHASH_OF_ARGS_2(TYPE, A0, A1) TYPEHASH_OF_ARGS_1(TYPE, A0) , Mem::InspectorInfo_Helper<decltype(TYPE :: A1)>::Info().m_typeHash
#define TYPEHASH_OF_ARGS_3(TYPE, A0, A1, A2) TYPEHASH_OF_ARGS_2(TYPE, A0, A1) , Mem::InspectorInfo_Helper<decltype(TYPE :: A2)>::Info().m_typeHash
#define TYPEHASH_OF_ARGS_4(TYPE, A0, A1, A2, A3) TYPEHASH_OF_ARGS_3(TYPE, A0, A1, A2) , Mem::InspectorInfo_Helper<decltype(TYPE :: A3)>::Info().m_typeHash
#define TYPEHASH_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) TYPEHASH_OF_ARGS_4(TYPE, A0, A1, A2, A3) , Mem::InspectorInfo_Helper<decltype(TYPE :: A4)>::Info().m_typeHash
#define TYPEHASH_OF_ARGS_6(TYPE, A0, A1, A2, A3, A4, A5) TYPEHASH_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) , Mem::InspectorInfo_Helper<decltype(TYPE :: A5)>::Info().m_typeHash

#define OFFSET_OF_ARGS_0(...)
#define OFFSET_OF_ARGS_1(TYPE, A0) offsetof(TYPE, A0)
#define OFFSET_OF_ARGS_2(TYPE, A0, A1) offsetof(TYPE, A0) , offsetof(TYPE, A1)
#define OFFSET_OF_ARGS_3(TYPE, A0, A1, A2) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2)
#define OFFSET_OF_ARGS_4(TYPE, A0, A1, A2, A3) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3)
#define OFFSET_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3) , offsetof(TYPE, A4)
#define OFFSET_OF_ARGS_6(TYPE, A0, A1, A2, A3, A4, A5) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3) , offsetof(TYPE, A4) , offsetof(TYPE, A5)

#define MakeInspectorInfo(TYPE, NUM_MEMBERS, ...) Mem::InspectorInfo(typeid(TYPE), { DEFER(STRINGIFY_ARGS_##NUM_MEMBERS( __VA_ARGS__ )) },\
	{ DEFER(TYPEHASH_OF_ARGS_##NUM_MEMBERS(TYPE, __VA_ARGS__)) }, { DEFER(OFFSET_OF_ARGS_##NUM_MEMBERS(TYPE, __VA_ARGS__)) });
