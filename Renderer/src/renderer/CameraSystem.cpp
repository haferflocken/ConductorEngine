#include <renderer/CameraSystem.h>

#include <renderer/ViewIDs.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace Renderer
{
void CameraSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	const bool homogeneousDepth = bgfx::getCaps()->homogeneousDepth;
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		const auto& cameraComponent = ecsGroup.Get<const CameraComponent>();

		float viewMatrix[16];
		bx::mtxInverse(viewMatrix, transformComponent.m_matrix.GetData());

		float projectionMatrix[16];
		bx::mtxProj(projectionMatrix, cameraComponent.m_verticalFieldOfView, m_aspectRatio, 0.1f, 100.0f,
			homogeneousDepth);

		bgfx::setViewTransform(cameraComponent.m_viewID, viewMatrix, projectionMatrix);
		bgfx::setViewRect(cameraComponent.m_viewID, 0, 0, m_widthPixels, m_heightPixels);
	}

	// Set the UI view to an orthographic view covering the space from (0, 0, 0) to (1, 1, 1).
	float uiProjectionMatrix[16];
	bx::mtxOrtho(uiProjectionMatrix, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1000.0f, 0.0f, homogeneousDepth);

	bgfx::setViewName(k_uiViewID, "UI View");
	bgfx::setViewTransform(k_uiViewID, nullptr, uiProjectionMatrix);
	bgfx::setViewRect(k_uiViewID, 0, 0, m_widthPixels, m_heightPixels);
}
}
