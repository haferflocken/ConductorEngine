#include <renderer/mesh/Vertex.h>

#include <bgfx/bgfx.h>

namespace Renderer::Mesh
{
const bgfx::VertexDecl& Vertex::GetVertexDecl()
{
	static bgfx::VertexDecl s_vertexDecl = []()
	{
		bgfx::VertexDecl decl;
		decl.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 3, bgfx::AttribType::Uint8)
			.end();
		return decl;
	}();

	return s_vertexDecl;
}
}
