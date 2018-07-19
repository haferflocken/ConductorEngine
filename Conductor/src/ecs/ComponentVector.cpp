#include <ecs/Component.h>
#include <ecs/ComponentFactory.h>
#include <ecs/ComponentID.h>
#include <ecs/ComponentVector.h>

#include <algorithm>

void ECS::ComponentVector::Remove(const ComponentID id, const ComponentFactory& componentFactory)
{
	const auto itr = std::lower_bound(begin(), end(), id,
		[](const Component& component, const ComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == end() || itr->m_id != id)
	{
		return;
	}

	Component& last = (*this)[m_count - 1];
	componentFactory.SwapComponents(*itr, last);

	componentFactory.DestroyComponent(last);
	m_count -= 1;
}
