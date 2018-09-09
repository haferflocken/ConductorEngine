#pragma once

#include <util/VariadicUtil.h>

namespace ECS::SystemUtil
{
template <typename SeenTypeList, typename... SystemTypes>
struct AreSystemsWriteCompatibleStruct;

template <typename SeenTypeList, typename TestType>
struct AreSystemsWriteCompatibleStruct<SeenTypeList, TestType>
{
	static constexpr bool value = TestType::IsWriteCompatibleWithAll(SeenTypeList());
};

template <typename SeenTypeList, typename TestType, typename... NextTypes>
struct AreSystemsWriteCompatibleStruct<SeenTypeList, TestType, NextTypes...>
{
	using AllOtherTypesList = typename SeenTypeList::template AppendType<NextTypes...>;
	
	static constexpr bool k_writeCompatible = TestType::IsWriteCompatibleWithAll(AllOtherTypesList());
	static constexpr bool k_rest = AreSystemsWriteCompatibleStruct<typename SeenTypeList::template AppendType<TestType>, NextTypes...>::value;

	static constexpr bool value = k_writeCompatible && k_rest;
};

/**
 * Test if a set of systems is write-compatible.
 * This means that none of the systems write to a component that another system reads or writes
 * and none of the systems read or write directly to entities.
 */
template <typename... SystemTypes>
inline constexpr bool AreSystemsWriteCompatible()
{
	return AreSystemsWriteCompatibleStruct<Util::TypeList<>, SystemTypes...>::value;
}
}
