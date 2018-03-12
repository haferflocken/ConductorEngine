#include <behave/ActorComponent.h>
#include <behave/ActorComponentFactory.h>
#include <behave/ActorComponentID.h>
#include <behave/ActorComponentVector.h>

#include <algorithm>

using namespace Behave;

void ActorComponentVector::Remove(const ActorComponentID id, const ActorComponentFactory& componentFactory)
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
