#pragma once

#include <Unit/CountUnits.h>

#include <type_traits>

namespace Behave
{
class Actor;
class ActorComponent;
class ActorManager;

/**
 * A group of strongly-typed Behave indices: these can be Actor indices or ActorComponent indices.
 * These are never constructed or destructed; they are created via casting the data within ActorComponentGroupVectors.
 */
template <typename... BehaveTypes>
class ActorComponentGroup
{
public:
	static constexpr size_t k_size{ sizeof...(BehaveTypes) };

	template <typename DesiredType>
	DesiredType& Get(ActorManager& actorManager) const
	{
		constexpr size_t Index = Util::IndexOfType<DesiredType, BehaveTypes...>;
		return ActorComponentGroupDetail::GetByIndexStruct<DesiredType, BehaveTypes...>::Get(actorManager, m_array[Index]);
	}

private:
	size_t m_array[k_size];
};

namespace ActorComponentGroupDetail
{
template <typename DesiredType, typename... BehaveTypes>
struct GetByIndexStruct;

template <typename... BehaveTypes>
struct GetByIndexStruct<Actor, BehaveTypes...>
{
	static Actor& Get(ActorManager& actorManager, const size_t index)
	{
		return actorManager.GetActorByIndex(index);
	}
};

template <typename ComponentType, typename... BehaveTypes>
struct GetByIndexStruct
{
	static std::enable_if_t<std::is_convertible_v<ComponentType&, ActorComponent&>, ComponentType>& Get(
		ActorManager& actorManager, const size_t index)
	{
		return static_cast<ComponentType&>(actorManager.GetComponentByIndex(ComponentType::Info::sk_typeHash, index));
	}
};
}
}
