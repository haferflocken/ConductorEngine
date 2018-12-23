#include <renderer/MeshSystem.h>

#include <renderer/ViewIDs.h>

namespace Renderer
{
MeshSystem::MeshSystem()
	: SystemTempl()
	, m_program()
	, m_vertexDecl()
	, m_staticMeshData()
{
	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);

	m_vertexDecl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8)
		.end();
}

MeshSystem::~MeshSystem()
{
	//bgfx::destroy(m_program);
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
	for (auto& entry : m_staticMeshData)
	{
		const Asset::AssetHandle<Mesh::StaticMesh>& handle = entry.first;
		MeshDatum& datum = entry.second;

		datum.m_timeSinceLastAccess += delta;

		const Mesh::StaticMesh* const mesh = handle.TryGetAsset();
		if (mesh == nullptr)
		{
			continue;
		}

		if (!bgfx::isValid(datum.m_vertexBuffer))
		{
			datum.m_vertexBuffer = bgfx::createVertexBuffer(
				bgfx::makeRef(&mesh->GetVertices().Front(), mesh->GetVertices().Size() * sizeof(Mesh::Vertex)),
				m_vertexDecl);
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
		auto& meshComponent = ecsGroup.Get<Mesh::MeshComponent>();

		const auto datumIter = m_staticMeshData.Find(meshComponent.m_meshHandle);
		if (datumIter == m_staticMeshData.end())
		{
			continue;
		}
		
		MeshDatum& datum = datumIter->second;
		datum.m_timeSinceLastAccess = Unit::Time::Millisecond(0);

		if ((!bgfx::isValid(datum.m_vertexBuffer)) || (!bgfx::isValid(datum.m_indexBuffer)))
		{
			continue;
		}

		encoder->setTransform(transformComponent.m_matrix.GetData());
		encoder->setVertexBuffer(0, datum.m_vertexBuffer);
		encoder->setIndexBuffer(datum.m_indexBuffer);
		encoder->setState(BGFX_STATE_DEFAULT);
		encoder->submit(k_sceneViewID, m_program);
	}

	// Discard any data for meshes that haven't been accessed recently.
	static constexpr Unit::Time::Millisecond k_discardAfterDuration{ 120000 };

	m_staticMeshData.RemoveAllMatching([](const auto& entry)
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
