#pragma once

#include <asset/AssetHandle.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <bgfx/bgfx.h>

#include <array>

namespace Asset { class AssetManager; }

namespace Image
{
struct ColourARGB;
class Pixel1Image;
}

namespace Math
{
class Frustum;
class Matrix4x4;
class Vector3;
}

namespace Mesh { struct PosColourVertex; }
namespace Renderer { class Shader; }

namespace Renderer::UI
{
/**
 * The TextRenderer creates and stores font meshes from Pixel1Images so that it can render text.
 */
class TextRenderer final
{
public:
	TextRenderer(const Math::Frustum& sceneViewFrustum, uint16_t widthPixels, uint16_t heightPixels);
	~TextRenderer();

	// Load the shaders this TextRenderer needs. 
	bool TryLoadShaders(Asset::AssetManager& assetManager);

	// Request that a font be created from the code page if it doesn't already exist.
	void RequestFont(const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const uint16_t characterWidthPixels,
		const uint16_t characterHeightPixels);

	// Submit text for rendering. This will silently fail if the font for the code page is unavailable.
	void SubmitText(bgfx::Encoder& encoder,
		const bgfx::ViewId viewID,
		const Math::Matrix4x4& worldTransform,
		const Image::ColourARGB colour,
		const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const char* const text,
		const float fontVerticalScale) const;

	void SubmitCameraFacingText(bgfx::Encoder& encoder,
		const bgfx::ViewId viewID,
		const Math::Vector3& position,
		const Image::ColourARGB colour,
		const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const char* const text,
		const float fontVerticalScale) const;

private:
	struct FontMeshDatum
	{
		FontMeshDatum();
		~FontMeshDatum();

		float m_characterWidthOverHeight{ 1.0f };

		bgfx::VertexBufferHandle m_glyphVertexBufferHandle{ BGFX_INVALID_HANDLE };
		std::array<bgfx::IndexBufferHandle, 256> m_glyphIndexBufferHandles;

		// A grid of vertices where each vertex is the corner of a pixel.
		Collection::Vector<Mesh::PosColourVertex> m_vertexGrid;

		// Index buffers for each glyph that create triangles for pixels.
		std::array<Collection::Vector<uint16_t>, 256> m_glyphIndexBuffers;
	};

	void CreateFontMeshFromImage(const Image::Pixel1Image& image,
		const uint16_t characterWidthPixels,
		const uint16_t characterHeightPixels,
		FontMeshDatum& font) const;

	void SubmitCharacterQuad(bgfx::Encoder& encoder,
		const bgfx::ViewId viewID,
		const FontMeshDatum& font,
		const Math::Matrix4x4& uiTransform,
		const Image::ColourARGB colour,
		const float characterWidth,
		const float characterHeight,
		const int x,
		const int y,
		const char c) const;

private:
	const Math::Frustum& m_sceneViewFrustum;

	uint16_t m_widthPixels;
	uint16_t m_heightPixels;

	Asset::AssetHandle<Shader> m_vertexShader;
	Asset::AssetHandle<Shader> m_fragmentShader;
	bgfx::ProgramHandle m_program;
	bgfx::UniformHandle m_colourUniform;
	Collection::VectorMap<Asset::AssetHandle<Image::Pixel1Image>, FontMeshDatum> m_fontMeshData;
};
}
