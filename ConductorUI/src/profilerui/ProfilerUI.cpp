#include <profilerui/ProfilerUI.h>

#include <condui/FontInfo.h>
#include <ecs/EntityManager.h>
#include <profilerui/ProfilerRootComponent.h>
#include <scene/SceneTransformComponent.h>

ECS::Entity& ProfilerUI::CreateProfilerEntity(ECS::EntityManager& entityManager,
	const float width, const float height, const float textHeight, const Condui::FontInfo& font)
{
	const auto componentTypes = { Scene::SceneTransformComponent::k_type, ProfilerRootComponent::k_type };
	ECS::Entity& profilerRootEntity = entityManager.CreateEntityWithComponents(
		{ componentTypes.begin(), componentTypes.size() }, ECS::EntityFlags::None, ECS::EntityLayers::k_conduiLayer);

	auto& profilerRootComponent = *entityManager.FindComponent<ProfilerRootComponent>(profilerRootEntity);
	profilerRootComponent.m_fontInfo = font;

	profilerRootComponent.m_width = width;
	profilerRootComponent.m_height = height;
	profilerRootComponent.m_textHeight = textHeight;

	return profilerRootEntity;
}
