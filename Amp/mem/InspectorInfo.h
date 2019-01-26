#pragma once

#include <collection/Vector.h>

#include <cstdint>
#include <initializer_list>

namespace Mem
{
struct InspectorInfo
{
	struct MemberInfo
	{
		const char* m_name;
		uint32_t m_typeCode;
		uint32_t m_offset;
	};

	const char* m_name;
	Collection::Vector<MemberInfo> m_memberInfo;
};

template <typename T>
uint32_t InspectorTypeCode()
{
	// TODO(info) inspector type codes
	return 0;
}

InspectorInfo MakeInspectorInfo_Internal(const char* name,
	std::initializer_list<const char*> memberNames,
	std::initializer_list<uint32_t> memberTypeCodes,
	std::initializer_list<size_t> memberOffsets);
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

#define TYPECODE_OF_ARGS_0(...)
#define TYPECODE_OF_ARGS_1(TYPE, A0) Mem::InspectorTypeCode<decltype(TYPE :: A0)>()
#define TYPECODE_OF_ARGS_2(TYPE, A0, A1) TYPECODE_OF_ARGS_1(TYPE, A0) , Mem::InspectorTypeCode<decltype(TYPE :: A1)>()
#define TYPECODE_OF_ARGS_3(TYPE, A0, A1, A2) TYPECODE_OF_ARGS_2(TYPE, A0, A1) , Mem::InspectorTypeCode<decltype(TYPE :: A2)>()
#define TYPECODE_OF_ARGS_4(TYPE, A0, A1, A2, A3) TYPECODE_OF_ARGS_3(TYPE, A0, A1, A2) , Mem::InspectorTypeCode<decltype(TYPE :: A3)>()
#define TYPECODE_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) TYPECODE_OF_ARGS_4(TYPE, A0, A1, A2, A3) , Mem::InspectorTypeCode<decltype(TYPE :: A4)>()
#define TYPECODE_OF_ARGS_6(TYPE, A0, A1, A2, A3, A4, A5) TYPECODE_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) , Mem::InspectorTypeCode<decltype(TYPE :: A5)>()

#define OFFSET_OF_ARGS_0(...)
#define OFFSET_OF_ARGS_1(TYPE, A0) offsetof(TYPE, A0)
#define OFFSET_OF_ARGS_2(TYPE, A0, A1) offsetof(TYPE, A0) , offsetof(TYPE, A1)
#define OFFSET_OF_ARGS_3(TYPE, A0, A1, A2) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2)
#define OFFSET_OF_ARGS_4(TYPE, A0, A1, A2, A3) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3)
#define OFFSET_OF_ARGS_5(TYPE, A0, A1, A2, A3, A4) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3) , offsetof(TYPE, A4)
#define OFFSET_OF_ARGS_6(TYPE, A0, A1, A2, A3, A4, A5) offsetof(TYPE, A0) , offsetof(TYPE, A1) , offsetof(TYPE, A2) , offsetof(TYPE, A3) , offsetof(TYPE, A4) , offsetof(TYPE, A5)

#define MakeInspectorInfo(TYPE, NUM_MEMBERS, ...) Mem::MakeInspectorInfo_Internal(#TYPE , { DEFER(STRINGIFY_ARGS_##NUM_MEMBERS ( __VA_ARGS__ )) },\
	{ DEFER(TYPECODE_OF_ARGS_##NUM_MEMBERS(TYPE, __VA_ARGS__)) }, { DEFER(OFFSET_OF_ARGS_##NUM_MEMBERS(TYPE, __VA_ARGS__)) })
