#include <profilerui/ProfilerThreadSystem.h>

#include <condui/Condui.h>
#include <dev/Profiler.h>
#include <ecs/EntityManager.h>
#include <scene/SceneTransformComponent.h>

namespace ProfilerUI
{
void ProfilerThreadSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// There's no need to defer a function if the function will do nothing.
	if (ecsGroups.Size() > 0)
	{
		// Defer the update to get access to the entity manager.
		deferredFunctions.Add(
			[this, ecsGroups](ECS::EntityManager& entityManager) { DeferredUpdate(entityManager, ecsGroups); });
	}
}

void ProfilerThreadSystem::DeferredUpdate(
	ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups) const
{
	Dev::Profiler::ForEachFrameRecordMap(
		[&](const uint64_t profilerThreadID, const Dev::Profiler::FrameRecordMap& frameRecordMap)
		{
			for (const auto& ecsGroup : ecsGroups)
			{
				const auto& profilerThreadComponent = ecsGroup.Get<const ProfilerThreadComponent>();
				if (profilerThreadComponent.m_profilerThreadID != profilerThreadID)
				{
					continue;
				}

				auto& entity = ecsGroup.Get<ECS::Entity>();
				// TODO(profiler) 
			}
		});
}
}