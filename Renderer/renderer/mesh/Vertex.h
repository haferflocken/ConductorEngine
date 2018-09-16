#pragma once

#include <cstdint>

namespace bgfx { struct VertexDecl; }

namespace Renderer::Mesh
{
struct Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static const bgfx::VertexDecl& GetVertexDecl();
};
}
