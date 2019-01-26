#include <scene/SceneTransformComponent.h>

#include <mem/InspectorInfo.h>

const ECS::ComponentType Scene::SceneTransformComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo Scene::SceneTransformComponent::k_inspectorInfo = MakeInspectorInfo(
	Scene::SceneTransformComponent, 2, m_modelToWorldMatrix, m_childToParentMatrix);
