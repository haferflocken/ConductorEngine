#include <renderer/MeshSystem.h>

#include <asset/AssetManager.h>
#include <renderer/VertexDeclarations.h>
#include <renderer/ViewIDs.h>

namespace Renderer
{
MeshSystem::MeshSystem(Asset::AssetManager& assetManager)
	: SystemTempl()
	, m_staticMeshVertexShader(assetManager.RequestAsset<Shader>(
		File::MakePath("shaders\\vs_static_mesh.bin"), Asset::LoadingMode::Immediate))
	, m_riggedMeshVertexShader(assetManager.RequestAsset<Shader>(
		File::MakePath("shaders\\vs_rigged_mesh.bin"), Asset::LoadingMode::Immediate))
	, m_fragmentShader(assetManager.RequestAsset<Shader>(
		File::MakePath("shaders\\fs_static_mesh.bin"), Asset::LoadingMode::Immediate))
	, m_staticMeshProgram(BGFX_INVALID_HANDLE)
	, m_riggedMeshProgram(BGFX_INVALID_HANDLE)
	, m_boneMatricesUniform(bgfx::createUniform("u_boneMatrices", bgfx::UniformType::Mat4, k_maxBones))
	, m_meshMetadata()
{
	const Shader* const fragmentShader = m_fragmentShader.TryGetAsset();
	if (fragmentShader != nullptr)
	{
		const Shader* const staticMeshVertexShader = m_staticMeshVertexShader.TryGetAsset();
		if (staticMeshVertexShader != nullptr)
		{
			m_staticMeshProgram = bgfx::createProgram(
				staticMeshVertexShader->GetShaderHandle(), fragmentShader->GetShaderHandle(), false);
		}
		const Shader* const riggedMeshVertexShader = m_riggedMeshVertexShader.TryGetAsset();
		if (riggedMeshVertexShader != nullptr)
		{
			m_riggedMeshProgram = bgfx::createProgram(
				riggedMeshVertexShader->GetShaderHandle(), fragmentShader->GetShaderHandle(), false);
		}
	}
}

MeshSystem::~MeshSystem()
{
	if (bgfx::isValid(m_riggedMeshProgram))
	{
		bgfx::destroy(m_riggedMeshProgram);
	}
	if (bgfx::isValid(m_staticMeshProgram))
	{
		bgfx::destroy(m_staticMeshProgram);
	}
}

void MeshSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	// Create vertex buffers and index buffers for any meshes that finished loading.
	for (auto& entry : m_meshMetadata)
	{
		const Asset::AssetHandle<Mesh::TriangleMesh>& handle = entry.first;
		MeshDatum& datum = entry.second;

		datum.m_timeSinceLastAccess += delta;

		const Mesh::TriangleMesh* const mesh = handle.TryGetAsset();
		if (mesh == nullptr)
		{
			continue;
		}

		if (!bgfx::isValid(datum.m_vertexBuffer))
		{
			const Mesh::CompactVertexDeclaration& vertexDeclaration = mesh->GetVertexDeclaration();
			const Mesh::ExpandedVertexDeclaration expandedDeclaration = vertexDeclaration.Expand();
			const bgfx::VertexDecl bgfxVertexDecl = MakeBGFXVertexDecl(expandedDeclaration);

			datum.m_vertexBuffer = bgfx::createVertexBuffer(
				bgfx::makeRef(&mesh->GetVertexData().Front(), mesh->GetVertexData().Size()),
				bgfxVertexDecl);
		}

		if (!bgfx::isValid(datum.m_indexBuffer))
		{
			datum.m_indexBuffer = bgfx::createIndexBuffer(
				bgfx::makeRef(&mesh->GetTriangleIndices().Front(), mesh->GetTriangleIndices().Size() * sizeof(uint16_t)));
		}

		if (!bgfx::isValid(datum.m_program))
		{
			const Mesh::CompactVertexDeclaration& vertexDeclaration = mesh->GetVertexDeclaration();
			datum.m_program = (vertexDeclaration.HasAttribute(Mesh::VertexAttribute::BoneWeights))
				? m_riggedMeshProgram : m_staticMeshProgram;
		}
	}

	// Render all the meshes.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		auto& meshComponent = ecsGroup.Get<Mesh::MeshComponent>();

		const auto datumIter = m_meshMetadata.Find(meshComponent.m_meshHandle);
		if (datumIter == m_meshMetadata.end())
		{
			// If the mesh isn't in m_meshMetadata, it may have changed since the entity was added.
			// Add it to m_meshMetadata so it gets picked up for the next update.
			m_meshMetadata[meshComponent.m_meshHandle];
			continue;
		}

		MeshDatum& datum = datumIter->second;
		const Mesh::TriangleMesh* const mesh = meshComponent.m_meshHandle.TryGetAsset();
		datum.m_timeSinceLastAccess = Unit::Time::Millisecond(0);

		if ((!bgfx::isValid(datum.m_vertexBuffer)) || (!bgfx::isValid(datum.m_indexBuffer)) ||
			(!bgfx::isValid(datum.m_program)) || mesh == nullptr)
		{
			continue;
		}

		// Rigged meshes need their bones as a uniform.
		if (datum.m_program.idx == m_riggedMeshProgram.idx)
		{
			// Memcpy to the stack so we don't over/underfill the uniform data.
			Math::Matrix4x4 boneMatrices[k_maxBones];

			const size_t numBoneMatrixBytes = meshComponent.m_boneToWorldMatrices.Size() * sizeof(Math::Matrix4x4);
			memcpy(boneMatrices, meshComponent.m_boneToWorldMatrices.begin(),
				std::min<size_t>(sizeof(boneMatrices), numBoneMatrixBytes));

			encoder->setUniform(m_boneMatricesUniform, boneMatrices, k_maxBones);
		}

		encoder->setTransform(transformComponent.m_modelToWorldMatrix.GetData());
		encoder->setVertexBuffer(0, datum.m_vertexBuffer);
		encoder->setIndexBuffer(datum.m_indexBuffer);
		encoder->setState(BGFX_STATE_DEFAULT);
		encoder->submit(k_sceneViewID, datum.m_program);
	}

	// Discard any data for meshes that haven't been accessed recently.
	static constexpr Unit::Time::Millisecond k_discardAfterDuration{ 120000 };

	m_meshMetadata.RemoveAllMatching([](const auto& entry)
		{
			const MeshDatum& datum = entry.second;
			return (datum.m_timeSinceLastAccess > k_discardAfterDuration);
		});

	bgfx::end(encoder);
}

void MeshSystem::NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group)
{
	// Create an entry in the mesh data for the entity's mesh handle.
	auto& meshComponent = group.Get<Mesh::MeshComponent>();
	m_meshMetadata[meshComponent.m_meshHandle];
}

void MeshSystem::NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group)
{
}

MeshSystem::MeshDatum::MeshDatum()
{
}

MeshSystem::MeshDatum::~MeshDatum()
{
	if (bgfx::isValid(m_vertexBuffer))
	{
		bgfx::destroy(m_vertexBuffer);
	}
	if (bgfx::isValid(m_indexBuffer))
	{
		bgfx::destroy(m_indexBuffer);
	}
}
}
