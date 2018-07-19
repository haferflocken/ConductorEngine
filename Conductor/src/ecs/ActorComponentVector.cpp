#include <ecs/ActorComponent.h>
#include <ecs/ActorComponentFactory.h>
#include <ecs/ActorComponentID.h>
#include <ecs/ActorComponentVector.h>

#include <algorithm>

void ECS::ActorComponentVector::Remove(const ActorComponentID id, const ActorComponentFactory& componentFactory)
{
	const auto itr = std::lower_bound(begin(), end(), id,
		[](const ActorComponent& component, const ActorComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == end() || itr->m_id != id)
	{
		return;
	}

	ActorComponent& last = (*this)[m_count - 1];
	componentFactory.SwapComponents(*itr, last);

	componentFactory.DestroyComponent(last);
	m_count -= 1;
}
