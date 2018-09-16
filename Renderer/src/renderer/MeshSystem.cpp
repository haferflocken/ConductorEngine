#include <renderer/MeshSystem.h>

namespace Renderer
{
MeshSystem::MeshSystem()
	: SystemTempl()
{
	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);
}

MeshSystem::~MeshSystem()
{
	//bgfx::destroy(m_program);
}

void MeshSystem::Update(ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions) const
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>(entityManager);
		auto& meshComponent = ecsGroup.Get<MeshComponent>(entityManager);

		const Mesh::StaticMesh* const mesh = meshComponent.m_meshHandle.TryGetAsset();
		if (mesh == nullptr)
		{
			if (bgfx::isValid(meshComponent.m_vertexBuffer))
			{
				bgfx::destroy(meshComponent.m_vertexBuffer);
				meshComponent.m_vertexBuffer = BGFX_INVALID_HANDLE;
			}
			if (bgfx::isValid(meshComponent.m_indexBuffer))
			{
				bgfx::destroy(meshComponent.m_indexBuffer);
				meshComponent.m_indexBuffer = BGFX_INVALID_HANDLE;
			}
			continue;
		}

		if (!bgfx::isValid(meshComponent.m_vertexBuffer))
		{
			meshComponent.m_vertexBuffer = bgfx::createVertexBuffer(
				bgfx::makeRef(&mesh->GetVertices().Front(), mesh->GetVertices().Size() * sizeof(Mesh::Vertex)),
				Mesh::Vertex::GetVertexDecl());
		}

		if (!bgfx::isValid(meshComponent.m_indexBuffer))
		{
			meshComponent.m_indexBuffer = bgfx::createIndexBuffer(
				bgfx::makeRef(&mesh->GetTriangleIndices().Front(), mesh->GetTriangleIndices().Size() * sizeof(uint16_t)));
		}
		
		encoder->setTransform(transformComponent.m_matrix.GetData());
		encoder->setVertexBuffer(0, meshComponent.m_vertexBuffer);
		encoder->setIndexBuffer(meshComponent.m_indexBuffer);
		encoder->setState(BGFX_STATE_DEFAULT);
		encoder->submit(0, m_program);
	}

	bgfx::end(encoder);
}
}
