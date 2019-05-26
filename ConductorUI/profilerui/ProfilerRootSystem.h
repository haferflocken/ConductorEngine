#pragma once

#include <ecs/Entity.h>
#include <ecs/System.h>
#include <profilerui/ProfilerRootComponent.h>

namespace ProfilerUI
{
/**
 * The ProfilerRootSystem creates child profiling thread UI entities for the first entity with a ProfilerRootComponent.
 */
class ProfilerRootSystem final : public ECS::SystemTempl<
	Util::TypeList<ProfilerRootComponent>,
	Util::TypeList<ECS::Entity>>
{
public:
	ProfilerRootSystem() = default;
	virtual ~ProfilerRootSystem() {}

	void Update(
		const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;

private:
	void DeferredUpdate(ECS::EntityManager& entityManager, const Collection::ArrayView<ECSGroupType>& ecsGroups) const;
};
}
