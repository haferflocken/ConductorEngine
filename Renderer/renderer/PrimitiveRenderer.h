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

// Draws a pyramid centered at the transform from (-0.5, -0.5) to (0.5, 0.5).
void DrawPyramid(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const Math::Vector3& scale,
	const Image::ColourARGB colour);

// Draws a cube centered at the transform from (-0.5, -0.5) to (0.5, 0.5).
void DrawCube(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Matrix4x4& transform,
	const Math::Vector3& scale,
	const Image::ColourARGB colour);

// Draws a pipe from the start position to the end position with the given radius.
void DrawPipe(bgfx::Encoder& encoder,
	const bgfx::ViewId viewID,
	const Math::Vector3& startPos,
	const Math::Vector3& endPos,
	const float radius,
	const Image::ColourARGB colour);
}
