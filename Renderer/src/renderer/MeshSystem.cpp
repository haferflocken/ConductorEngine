#include <renderer/MeshSystem.h>

namespace Renderer
{
MeshSystem::MeshSystem()
	: SystemTempl()
	, m_program()
	, m_staticMeshData()
{
	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);
}

MeshSystem::~MeshSystem()
{
	//bgfx::destroy(m_program);
}

void MeshSystem::Update(const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	// Discard any data for meshes that haven't been accessed recently.
	static constexpr float k_discardAfterSeconds = 120.0f;

	m_staticMeshData.RemoveAllMatching([](const auto& entry)
		{
			const MeshDatum& datum = entry.second;
			return (datum.m_secondsSinceAccess > k_discardAfterSeconds);
		});

	// Create vertex buffers and index buffers for any meshes that finished loading.
	for (auto& entry : m_staticMeshData)
	{
		const Asset::AssetHandle<Mesh::StaticMesh>& handle = entry.first;
		MeshDatum& datum = entry.second;

		const Mesh::StaticMesh* const mesh = handle.TryGetAsset();
		if (mesh == nullptr)
		{
			continue;
		}

		if (!bgfx::isValid(datum.m_vertexBuffer))
		{
			datum.m_vertexBuffer = bgfx::createVertexBuffer(
				bgfx::makeRef(&mesh->GetVertices().Front(), mesh->GetVertices().Size() * sizeof(Mesh::Vertex)),
				Mesh::Vertex::GetVertexDecl());
		}

		if (!bgfx::isValid(datum.m_indexBuffer))
		{
			datum.m_indexBuffer = bgfx::createIndexBuffer(
				bgfx::makeRef(&mesh->GetTriangleIndices().Front(), mesh->GetTriangleIndices().Size() * sizeof(uint16_t)));
		}
	}

	// Render all the meshes.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		auto& meshComponent = ecsGroup.Get<MeshComponent>();

		const auto datumIter = m_staticMeshData.Find(meshComponent.m_meshHandle);
		if (datumIter == m_staticMeshData.end())
		{
			continue;
		}
		
		encoder->setTransform(transformComponent.m_matrix.GetData());
		encoder->setVertexBuffer(0, datumIter->second.m_vertexBuffer);
		encoder->setIndexBuffer(datumIter->second.m_indexBuffer);
		encoder->setState(BGFX_STATE_DEFAULT);
		encoder->submit(0, m_program);
	}

	bgfx::end(encoder);
}

void MeshSystem::NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group)
{
	// Create an entry in the mesh data for the entity's mesh handle.
	auto& meshComponent = group.Get<MeshComponent>();
	m_staticMeshData[meshComponent.m_meshHandle];
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
