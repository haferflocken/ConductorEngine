#pragma once

#include <collection/VectorMap.h>
#include <ecs/EntityID.h>
#include <ecs/System.h>
#include <mesh/MeshComponent.h>
#include <mesh/MeshComponentInfo.h>
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
	Util::TypeList<Mesh::MeshComponent>,
	ECS::SystemBindingType::Extended>
{
public:
	MeshSystem();
	~MeshSystem();

	void Update(const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

	void NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group);
	void NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group);

private:
	struct MeshDatum
	{
		MeshDatum();
		~MeshDatum();

		// Vertex and index buffer handles for use with bgfx.
		bgfx::VertexBufferHandle m_vertexBuffer{ BGFX_INVALID_HANDLE };
		bgfx::IndexBufferHandle m_indexBuffer{ BGFX_INVALID_HANDLE };
		// The number of seconds since this mesh was last accessed.
		// Data for meshes that have not been accessed in a while is discarded.
		float m_secondsSinceAccess{ 0 };
	};

	bgfx::ProgramHandle m_program;
	bgfx::VertexDecl m_vertexDecl;
	Collection::VectorMap<Asset::AssetHandle<Mesh::StaticMesh>, MeshDatum> m_staticMeshData;
};
}
