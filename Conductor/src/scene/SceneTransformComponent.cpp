#include <scene/SceneTransformComponent.h>

#include <mem/InspectorInfo.h>

const ECS::ComponentType Scene::SceneTransformComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash Scene::SceneTransformComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(
	Scene::SceneTransformComponent, 2, m_modelToWorldMatrix, m_childToParentMatrix);
