#pragma once

#include <bgfx/bgfx.h>

namespace Renderer
{
const bgfx::VertexDecl k_staticMeshVertexDecl = []()
{
	bgfx::VertexDecl staticMeshVertexDecl;
	staticMeshVertexDecl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();
	return staticMeshVertexDecl;
}();
}
