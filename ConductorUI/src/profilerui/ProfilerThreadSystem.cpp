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
#if AMP_PROFILING_ENABLED == 1
	// There's no need to defer a function if the function will do nothing.
	if (ecsGroups.Size() > 0)
	{
		// A system can't create entities while it's iterating over ECS groups because creating an entity updates
		// the EntityManager's ECS groups. To work around this, we make a copy of the ECS groups, which works because
		// entities and components don't move around in memory.
		Collection::Vector<ECSGroupType> copiedGroups(static_cast<uint32_t>(ecsGroups.Size()));
		copiedGroups.Resize(ecsGroups.Size());
		memcpy(copiedGroups.begin(), ecsGroups.begin(), ecsGroups.Size() * sizeof(ECSGroupType));

		// Defer the update to get access to the entity manager.
		deferredFunctions.Add(
			[this, groups = std::move(copiedGroups)](ECS::EntityManager& entityManager) mutable
			{
				const auto groupsView = groups.GetView();
				DeferredUpdate(entityManager, groups.GetView());
			});
}
#endif
}

void ProfilerThreadSystem::DeferredUpdate(
	ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups) const
{
#if AMP_PROFILING_ENABLED == 1
	Dev::Profiler::ForEachFrameRecordMap(
		[&](const uint64_t profilerThreadID, const Dev::Profiler::FrameRecordMap& frameRecordMap)
		{
			for (const auto& ecsGroup : ecsGroups)
			{
				auto& profilerThreadComponent = ecsGroup.Get<ProfilerThreadComponent>();
				if (profilerThreadComponent.m_profilerThreadID != profilerThreadID)
				{
					continue;
				}

				const float numMetresPerNanosecond = 1e-9f * 1.0f;
				const Condui::FontInfo* const fontInfo = &profilerThreadComponent.m_fontInfo;
				const float textHeight = profilerThreadComponent.m_textHeight;
				auto& entity = ecsGroup.Get<ECS::Entity>();

				const uint64_t numFrameRecords = frameRecordMap.GetNumFrameRecords();
				for (uint64_t i = profilerThreadComponent.m_nextFrameRecordID; i < numFrameRecords; ++i)
				{
					const auto& frameRecord = frameRecordMap.GetFrameRecord(i);
					if (frameRecord.m_durationNanoseconds == UINT64_MAX)
					{
						continue;
					}

					const float frameBeginY = frameRecord.m_depth * profilerThreadComponent.m_textHeight;
					const float frameBeginX = frameRecord.m_beginPoint * numMetresPerNanosecond;
					const float frameLengthX = frameRecord.m_durationNanoseconds * numMetresPerNanosecond;

					auto frameElement = Condui::MakeTextDisplayElement(
						frameLengthX, textHeight, frameRecord.m_frameName, textHeight);
					ECS::Entity& frameEntity = Condui::CreateConduiEntity(
						entityManager, std::move(frameElement), fontInfo);

					auto& childTransformComponent =
						*entityManager.FindComponent<Scene::SceneTransformComponent>(frameEntity);
					childTransformComponent.m_childToParentMatrix =
						Math::Matrix4x4::MakeTranslation(frameBeginX, frameBeginY, 0.0f);

					entityManager.SetParentEntity(frameEntity, &entity);
}
				profilerThreadComponent.m_nextFrameRecordID = numFrameRecords;
			}
		});
#endif
}
}