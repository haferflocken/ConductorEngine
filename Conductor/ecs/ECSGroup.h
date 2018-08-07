#pragma once

#include <unit/CountUnits.h>

#include <type_traits>

namespace ECS
{
class Component;
class Entity;
class EntityManager;

/**
 * A group of strongly-typed ECS indices: these can be Entity indices or Component indices.
 * These are never constructed or destructed; they are created via casting the data within ECSGroupVectors.
 */
template <typename... ECSTypes>
class ECSGroup
{
public:
	static constexpr size_t k_size{ sizeof...(ECSTypes) };

	template <typename DesiredType>
	DesiredType& Get(EntityManager& entityManager) const
	{
		constexpr size_t Index = Util::IndexOfType<DesiredType, ECSTypes...>;
		return ComponentGroupDetail::GetByIndexStruct<DesiredType, ECSTypes...>::Get(entityManager, m_array[Index]);
	}

private:
	size_t m_array[k_size];
};

namespace ComponentGroupDetail
{
template <typename DesiredType, typename... ECSTypes>
struct GetByIndexStruct;

template <typename... ECSTypes>
struct GetByIndexStruct<Entity, ECSTypes...>
{
	static Entity& Get(EntityManager& entityManager, const size_t index)
	{
		return entityManager.GetEntityByIndex(index);
	}
};

template <typename ComponentType, typename... ECSTypes>
struct GetByIndexStruct
{
	static std::enable_if_t<std::is_convertible_v<ComponentType&, const Component&>, ComponentType>& Get(
		EntityManager& entityManager, const size_t index)
	{
		return static_cast<ComponentType&>(entityManager.GetComponentByIndex(ComponentType::Info::sk_typeHash, index));
	}
};
}
}
