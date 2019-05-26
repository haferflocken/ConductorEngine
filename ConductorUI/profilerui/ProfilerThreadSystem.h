#pragma once

#include <ecs/Entity.h>
#include <ecs/System.h>
#include <profilerui/ProfilerThreadComponent.h>

namespace ProfilerUI
{
/**
 * The ProfilerThreadSystem creates and maintains Condui entities to show profiling data for a thread.
 */
class ProfilerThreadSystem final : public ECS::SystemTempl<
	Util::TypeList<ProfilerThreadComponent>,
	Util::TypeList<ECS::Entity>>
{
public:
	ProfilerThreadSystem() = default;
	virtual ~ProfilerThreadSystem() {}

	void Update(
		const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;

private:
	void DeferredUpdate(ECS::EntityManager& entityManager, const Collection::ArrayView<ECSGroupType>& ecsGroups) const;
};
}
