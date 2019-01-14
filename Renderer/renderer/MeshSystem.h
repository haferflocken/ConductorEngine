#pragma once

#include <asset/AssetHandle.h>
#include <collection/VectorMap.h>
#include <ecs/EntityID.h>
#include <ecs/System.h>
#include <mesh/MeshComponent.h>
#include <mesh/MeshComponentInfo.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>
#include <renderer/Shader.h>

#include <bgfx/bgfx.h>

namespace Asset { class AssetManager; }

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
	explicit MeshSystem(Asset::AssetManager& assetManager);
	~MeshSystem();

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
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
		// The number of milliseconds since this mesh was last accessed.
		// Data for meshes that have not been accessed in a while is discarded.
		Unit::Time::Millisecond m_timeSinceLastAccess{ 0 };
	};

	Asset::AssetHandle<Shader> m_vertexShader;
	Asset::AssetHandle<Shader> m_fragmentShader;
	bgfx::ProgramHandle m_program;
	Collection::VectorMap<Asset::AssetHandle<Mesh::StaticMesh>, MeshDatum> m_staticMeshData;
};
}
