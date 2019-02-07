#pragma once

#include <math/Matrix4x4.h>

#include <bgfx/bgfx.h>

namespace Asset { class AssetManager; }
namespace Image { struct ColourARGB; }

/**
 * PrimitiveRenderer contains helper draws lines, triangles, and quads.
 */
namespace Renderer::PrimitiveRenderer
{
bool Initialize(Asset::AssetManager& assetManager);
void Shutdown();

// Draws a quad from (0, 0) to (width, height) at the transform.
void DrawQuad(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const float width,
	const float height,
	const Image::ColourARGB colour);
}
