#include <renderer/CameraSystem.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace Renderer
{
void CameraSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	const bool homogeneousDepth = bgfx::getCaps()->homogeneousDepth;

	// Construct and apply a default camera for the scene view.
	{
		float viewMatrix[16];
		bx::mtxIdentity(viewMatrix);

		float projectionMatrix[16];
		bx::mtxProj(projectionMatrix, 60.0f, m_aspectRatio, 0.1f, 100.0f, homogeneousDepth);

		bgfx::setViewTransform(k_sceneViewID, viewMatrix, projectionMatrix);
		bgfx::setViewRect(k_sceneViewID, 0, 0, m_widthPixels, m_heightPixels);
	}

	// Loop over all cameras in the scene and apply them.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		const auto& cameraComponent = ecsGroup.Get<const CameraComponent>();

		float viewMatrix[16];
		bx::mtxInverse(viewMatrix, transformComponent.m_modelToWorldMatrix.GetData());

		float projectionMatrix[16];
		bx::mtxProj(projectionMatrix, cameraComponent.m_verticalFieldOfView, m_aspectRatio, 0.1f, 100.0f,
			homogeneousDepth);

		bgfx::setViewTransform(cameraComponent.m_viewID, viewMatrix, projectionMatrix);
		bgfx::setViewRect(cameraComponent.m_viewID, 0, 0, m_widthPixels, m_heightPixels);
	}
}
}
