#include <renderer/CameraSystem.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace Renderer
{
void CameraSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	// Construct and apply a default camera for the scene view.
	{
		constexpr float k_defaultNearDistance = 0.1f;
		constexpr float k_defaultFarDistance = 100.0f;
		constexpr float k_defaultVerticalFOV = 60.0f;

		SubmitCamera(
			k_sceneViewID, Math::Matrix4x4(), k_defaultNearDistance, k_defaultFarDistance, k_defaultVerticalFOV);

		m_sceneViewFrustum = Math::Frustum(Math::Matrix4x4(),
			k_defaultNearDistance,
			k_defaultFarDistance,
			k_defaultVerticalFOV * bx::kPi / 180.0f,
			m_aspectRatio);
	}

	// Loop over all cameras in the scene and apply them.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		const auto& cameraComponent = ecsGroup.Get<const CameraComponent>();

		SubmitCamera(cameraComponent.m_viewID,
			transformComponent.m_modelToWorldMatrix,
			cameraComponent.m_nearDistance,
			cameraComponent.m_farDistance,
			cameraComponent.m_verticalFieldOfView);

		if (cameraComponent.m_viewID == k_sceneViewID)
		{
			m_sceneViewFrustum = Math::Frustum(transformComponent.m_modelToWorldMatrix,
				cameraComponent.m_nearDistance,
				cameraComponent.m_farDistance,
				cameraComponent.m_verticalFieldOfView * bx::kPi / 180.0f,
				m_aspectRatio);
		}
	}
}

void CameraSystem::SubmitCamera(uint16_t viewID,
	const Math::Matrix4x4& cameraToWorldMatrix,
	float nearDistance,
	float farDistance,
	float verticalFOV) const
{
	const bool homogeneousDepth = bgfx::getCaps()->homogeneousDepth;

	float viewMatrix[16];
	bx::mtxInverse(viewMatrix, cameraToWorldMatrix.GetData());

	float projectionMatrix[16];
	bx::mtxProj(projectionMatrix, verticalFOV, m_aspectRatio, nearDistance, farDistance,
		homogeneousDepth);

	bgfx::setViewTransform(viewID, viewMatrix, projectionMatrix);
	bgfx::setViewRect(viewID, 0, 0, m_widthPixels, m_heightPixels);
}
}
