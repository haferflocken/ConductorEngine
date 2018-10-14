#pragma once

#include <ecs/ComponentType.h>
#include <unit/CountUnits.h>

#include <type_traits>

namespace ECS
{
class Component;
class Entity;

/**
 * A group of strongly-typed ECS pointers: these can be Entity pointers or Component pointers.
 * These are never constructed or destructed; they are created via casting the data within ECSGroupVectors.
 */
template <typename... ECSTypes>
class ECSGroup
{
public:
	static constexpr size_t k_size{ sizeof...(ECSTypes) };

	template <typename DesiredType>
	DesiredType& Get() const
	{
		constexpr size_t Index = Util::IndexOfType<DesiredType, ECSTypes...>;
		return *reinterpret_cast<DesiredType*>(m_array[Index]);
	}

private:
	void* m_array[k_size];
};
}
