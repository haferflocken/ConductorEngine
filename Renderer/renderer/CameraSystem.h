#pragma once

#include <renderer/CameraComponent.h>
#include <renderer/CameraComponentInfo.h>

#include <ecs/System.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Renderer
{
class CameraSystem final : public ECS::SystemTempl<
	Util::TypeList<Scene::SceneTransformComponent, CameraComponent>,
	Util::TypeList<>>
{
public:
	CameraSystem(uint16_t widthPixels, uint16_t heightPixels)
		: SystemTempl()
		, m_widthPixels(widthPixels)
		, m_heightPixels(heightPixels)
		, m_aspectRatio(static_cast<float>(widthPixels) / static_cast<float>(heightPixels))
	{}

	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;

private:
	uint16_t m_widthPixels;
	uint16_t m_heightPixels;
	float m_aspectRatio;
};
}
