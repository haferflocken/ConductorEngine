#pragma once

#include <renderer/MeshComponent.h>
#include <renderer/MeshComponentInfo.h>

#include <ecs/System.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

#include <bgfx/bgfx.h>

namespace Renderer
{
/**
 * Makes entities visible by rendering their MeshComponent.
 */
class MeshSystem final : public ECS::SystemTempl<
	Util::TypeList<Scene::SceneTransformComponent>,
	Util::TypeList<MeshComponent>>
{
public:
	MeshSystem();
	~MeshSystem();

	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;

private:
	bgfx::ProgramHandle m_program;
};
}
