#include <profilerui/ProfilerRootSystem.h>

#include <condui/Condui.h>
#include <dev/Profiler.h>
#include <ecs/EntityManager.h>
#include <profilerui/ProfilerThreadComponent.h>
#include <scene/SceneTransformComponent.h>

namespace ProfilerUI
{
void ProfilerRootSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
#if AMP_PROFILING_ENABLED == 1
	// There's no need to defer a function if the function will do nothing.
	if (ecsGroups.Size() > 0)
	{
		// Defer the update to get access to the entity manager.
		deferredFunctions.Add(
			[this, ecsGroups](ECS::EntityManager& entityManager) { DeferredUpdate(entityManager, ecsGroups); });
	}
#endif
}

void ProfilerRootSystem::DeferredUpdate(
	ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups) const
{
#if AMP_PROFILING_ENABLED == 1
	// This system only operates on the first entity it matches.
	auto& entity = ecsGroups[0].Get<ECS::Entity>();
	const auto& profilerRootComponent = ecsGroups[0].Get<const ProfilerRootComponent>();
	
	Collection::Vector<uint64_t> existingThreadIDs(static_cast<uint32_t>(entity.GetChildren().Size()));
	for (auto& child : entity.GetChildren())
	{
		auto* childComponent = entityManager.FindComponent<const ProfilerThreadComponent>(*child);
		if (childComponent != nullptr)
		{
			existingThreadIDs.Add(childComponent->m_profilerThreadID);
		}
	}

	Dev::Profiler::ForEachFrameRecordMap(
		[&](const uint64_t profilerThreadID, const Dev::Profiler::FrameRecordMap& frameRecordMap)
		{
			for (const auto& existingThreadID : existingThreadIDs)
			{
				if (existingThreadID == profilerThreadID)
				{
					// As this is a lambda, this just continues to the next profiler thread ID.
					return;
				}
			}

			const auto componentTypes = { Scene::SceneTransformComponent::k_type, ProfilerThreadComponent::k_type };
			ECS::Entity& childEntity = entityManager.CreateEntityWithComponents(
				{ componentTypes.begin(), componentTypes.size() },
				ECS::EntityFlags::None,
				ECS::EntityLayers::k_conduiLayer);

			auto& childComponent = *entityManager.FindComponent<ProfilerThreadComponent>(childEntity);
			childComponent.m_profilerThreadID = profilerThreadID;
			childComponent.m_fontInfo = profilerRootComponent.m_fontInfo;
			childComponent.m_textHeight = profilerRootComponent.m_textHeight;
		});
#endif
}
}
