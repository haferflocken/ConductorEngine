#include <renderer/CameraSystem.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace Renderer
{
void CameraSystem::Update(ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions) const
{
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>(entityManager);
		const auto& cameraComponent = ecsGroup.Get<const CameraComponent>(entityManager);

		float viewMatrix[16];
		bx::mtxInverse(viewMatrix, transformComponent.m_matrix.GetData());

		float projectionMatrix[16];
		bx::mtxProj(projectionMatrix, cameraComponent.m_verticalFieldOfView, m_aspectRatio, 0.1f, 100.0f,
			bgfx::getCaps()->homogeneousDepth);

		bgfx::setViewTransform(cameraComponent.m_viewID, viewMatrix, projectionMatrix);
		bgfx::setViewRect(cameraComponent.m_viewID, 0, 0, m_widthPixels, m_heightPixels);
	}
}
}
