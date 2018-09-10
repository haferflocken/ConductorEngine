#include <renderer/MeshSystem.h>

namespace Renderer
{
namespace Internal_MeshSystem
{
struct Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static bgfx::VertexDecl s_layout;
};

bgfx::VertexDecl Vertex::s_layout;

const Vertex k_vertices[] =
{
{ -1.0f, -1.0f, 0.0f, 0xff0000ff },
{ 1.0f, -1.0f, 0.0f, 0xffff0000 },
{ -1.0f, 1.0f, 0.0f, 0xff00ff00 },
{ 1.0f, 1.0f, 0.0f, 0xffffff00 }
};

const uint16_t k_triangleList[] =
{
	0, 1, 2,
	2, 1, 3
};
}

MeshSystem::MeshSystem()
	: SystemTempl()
{
	using namespace Internal_MeshSystem;

	Vertex::s_layout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
		.end();

	m_vertexBuffer = bgfx::createVertexBuffer(bgfx::makeRef(k_vertices, sizeof(k_vertices)), Vertex::s_layout);
	m_indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(k_triangleList, sizeof(k_triangleList)));
	
	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);
}

MeshSystem::~MeshSystem()
{
	//bgfx::destroy(m_program);
	bgfx::destroy(m_indexBuffer);
	bgfx::destroy(m_vertexBuffer);
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

		encoder->setTransform(transformComponent.m_matrix.GetData());
		encoder->setVertexBuffer(0, m_vertexBuffer);
		encoder->setIndexBuffer(m_indexBuffer);
		encoder->setState(BGFX_STATE_DEFAULT);
		encoder->submit(0, m_program);
	}

	bgfx::end(encoder);
}
}
