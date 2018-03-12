#pragma once

#include <Unit/CountUnits.h>

namespace Behave
{
class ActorComponent;

/**
 * A group of strongly-typed actor component pointers. These are never constructed or destructed; they are
 * created via casting the data within ActorComponentGroupVectors.
 */
template <typename... ComponentTypes>
class ActorComponentGroup
{
public:
	static constexpr size_t k_size{ sizeof...(ComponentTypes) };

	template <typename DesiredComponentType>
	DesiredComponentType& Get() const
	{
		constexpr size_t Index = Util::IndexOfType<DesiredComponentType, ComponentTypes...>;
		return static_cast<DesiredComponentType&>(*m_array[Index]);
	}

private:
	ActorComponent* m_array[k_size];
};
}
