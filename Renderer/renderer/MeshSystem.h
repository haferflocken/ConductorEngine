#pragma once

#include <asset/AssetHandle.h>
#include <collection/VectorMap.h>
#include <ecs/EntityID.h>
#include <ecs/System.h>
#include <mesh/MeshComponent.h>
#include <scene/SceneTransformComponent.h>
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
	static constexpr uint16_t k_maxBones = 32;
	static constexpr uint16_t k_maxWeightGroups = 32;

	struct MeshDatum
	{
		MeshDatum();
		~MeshDatum();

		// Vertex and index buffer handles for use with bgfx.
		bgfx::VertexBufferHandle m_vertexBuffer{ BGFX_INVALID_HANDLE };
		bgfx::IndexBufferHandle m_indexBuffer{ BGFX_INVALID_HANDLE };

		// The program handle for use with bgfx.
		bgfx::ProgramHandle m_program{ BGFX_INVALID_HANDLE };

		// The number of milliseconds since this mesh was last accessed.
		// Data for meshes that have not been accessed in a while is discarded.
		Unit::Time::Millisecond m_timeSinceLastAccess{ 0 };
	};

private:
	Asset::AssetHandle<Shader> m_staticMeshVertexShader;
	Asset::AssetHandle<Shader> m_riggedMeshVertexShader;
	Asset::AssetHandle<Shader> m_fragmentShader;
	bgfx::ProgramHandle m_staticMeshProgram;
	bgfx::ProgramHandle m_riggedMeshProgram;
	bgfx::UniformHandle m_boneMatricesUniform;
	bgfx::UniformHandle m_weightGroupsUniform;
	Collection::VectorMap<Asset::AssetHandle<Mesh::TriangleMesh>, MeshDatum> m_meshMetadata;
};
}
