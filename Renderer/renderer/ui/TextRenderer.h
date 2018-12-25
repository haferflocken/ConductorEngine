#pragma once

#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <bgfx/bgfx.h>

#include <array>

namespace Asset
{
template <typename TAsset>
class AssetHandle;
}
namespace Image { class Pixel1Image; }
namespace Math { class Matrix4x4; }
namespace Mesh { struct Vertex; }

namespace Renderer::UI
{
/**
 * The TextRenderer creates and stores font meshes from Pixel1Images so that it can render text.
 */
class TextRenderer final
{
public:
	TextRenderer(uint16_t widthPixels, uint16_t heightPixels);
	~TextRenderer();

	// Request that a font be created from the code page if it doesn't already exist.
	void RequestFont(const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const uint16_t characterWidthPixels,
		const uint16_t characterHeightPixels);

	// Submit text for rendering. This will silently fail if the font for the code page is unavailable.
	void SubmitText(bgfx::Encoder& encoder,
		const Math::Matrix4x4& uiTransform,
		const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const char* const text,
		const float fontScale) const;

private:
	struct FontMeshDatum
	{
		FontMeshDatum();
		~FontMeshDatum();

		uint16_t m_characterWidthPixels{ 0 };
		uint16_t m_characterHeightPixels{ 0 };

		bgfx::VertexBufferHandle m_glyphVertexBufferHandle{ BGFX_INVALID_HANDLE };
		std::array<bgfx::IndexBufferHandle, 256> m_glyphIndexBufferHandles;

		// A grid of vertices where each vertex is the corner of a pixel.
		Collection::Vector<Mesh::Vertex> m_vertexGrid;

		// Index buffers for each glyph that create triangles for pixels.
		std::array<Collection::Vector<uint16_t>, 256> m_glyphIndexBuffers;
	};

	void CreateFontMeshFromImage(const Image::Pixel1Image& image,
		FontMeshDatum& font) const;

	void SubmitCharacterQuad(bgfx::Encoder& encoder,
		const FontMeshDatum& font,
		const Math::Matrix4x4& uiTransform,
		const float characterWidth,
		const float characterHeight,
		const int x,
		const int y,
		const char c) const;

	uint16_t m_widthPixels;
	uint16_t m_heightPixels;

	bgfx::ProgramHandle m_program;
	bgfx::VertexDecl m_vertexDecl;
	Collection::VectorMap<Asset::AssetHandle<Image::Pixel1Image>, FontMeshDatum> m_fontMeshData;
};
}
