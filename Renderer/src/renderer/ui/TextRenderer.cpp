#include <renderer/ui/TextRenderer.h>

#include <asset/AssetHandle.h>
#include <image/Pixel1Image.h>
#include <math/Matrix4x4.h>
#include <mesh/Vertex.h>
#include <renderer/ViewIDs.h>

namespace Renderer::UI
{
TextRenderer::TextRenderer(uint16_t widthPixels, uint16_t heightPixels)
	: m_widthPixels(widthPixels)
	, m_heightPixels(heightPixels)
	, m_program()
	, m_vertexDecl()
	, m_fontMeshData()
{
	//m_program = bgfx::createProgram(vertexShader, fragmentShader, true);

	m_vertexDecl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8)
		.end();
}

TextRenderer::~TextRenderer()
{
	//bgfx::destroy(m_program);
}

void TextRenderer::RequestFont(const Asset::AssetHandle<Image::Pixel1Image>& codePageHandle,
	const uint16_t characterWidthPixels,
	const uint16_t characterHeightPixels)
{
	const Image::Pixel1Image* const codePage = codePageHandle.TryGetAsset();
	if (codePage == nullptr)
	{
		return;
	}

	FontMeshDatum& font = m_fontMeshData[codePageHandle];
	if (bgfx::isValid(font.m_glyphVertexBufferHandle))
	{
		AMP_ASSERT(characterWidthPixels == font.m_characterWidthPixels
			&& characterHeightPixels == font.m_characterHeightPixels,
			"Dimensions of a font can't be changed after it's created!");
		return;
	}
	font.m_characterWidthPixels = characterWidthPixels;
	font.m_characterHeightPixels = characterHeightPixels;
	CreateFontMeshFromImage(*codePage, font);
}

void TextRenderer::SubmitText(bgfx::Encoder& encoder,
	const Math::Matrix4x4& uiTransform,
	const Asset::AssetHandle<Image::Pixel1Image>& codePage,
	const char* const text,
	const float fontScale) const
{
	const auto datumIter = m_fontMeshData.Find(codePage);
	if (datumIter == m_fontMeshData.end())
	{
		return;
	}

	const FontMeshDatum& font = datumIter->second;
	if (!bgfx::isValid(font.m_glyphVertexBufferHandle))
	{
		return;
	}

	const float characterWidth = (fontScale * font.m_characterWidthPixels) / m_widthPixels;
	const float characterHeight = (fontScale * font.m_characterHeightPixels) / m_heightPixels;

	int x = 0;
	int y = -1; // We begin at -1 because characters render from the bottom up.
	char previous = '#';

	for (const char* c = text; *c != '\0'; ++c)
	{
		switch (*c)
		{
		case ' ':
		{
			x += 1;
			break;
		}
		case '\t':
		{
			const int xAt4 = x & ~(0b11);
			x = xAt4 + 4;
			break;
		}
		case '\r':
		{
			x = 0;
			y -= 1;
			break;
		}
		case '\n':
		{
			if (previous != '\r')
			{
				x = 0;
				y -= 1;
			}
			break;
		}
		default:
		{
			SubmitCharacterQuad(encoder,
				font,
				uiTransform,
				characterWidth,
				characterHeight,
				x,
				y,
				*c);

			x += 1;
			break;
		}
		}
		previous = *c;
	}
}

TextRenderer::FontMeshDatum::FontMeshDatum()
{
	std::fill(m_glyphIndexBufferHandles.begin(), m_glyphIndexBufferHandles.end(),
		bgfx::IndexBufferHandle(BGFX_INVALID_HANDLE));
}

TextRenderer::FontMeshDatum::~FontMeshDatum()
{
	if (bgfx::isValid(m_glyphVertexBufferHandle))
	{
		bgfx::destroy(m_glyphVertexBufferHandle);
	}
	for (auto& handle : m_glyphIndexBufferHandles)
	{
		if (bgfx::isValid(handle))
		{
			bgfx::destroy(handle);
		}
	}
}

void TextRenderer::CreateFontMeshFromImage(const Image::Pixel1Image& image,
	FontMeshDatum& font) const
{
	const uint32_t characterWidthPixels = font.m_characterWidthPixels;
	const uint32_t characterHeightPixels = font.m_characterHeightPixels;

	const uint32_t numColumns = image.GetWidth() / characterWidthPixels;
	const uint32_t numRows = image.GetHeight() / characterHeightPixels;
	if ((numColumns * numRows) != font.m_glyphIndexBufferHandles.size())
	{
		return;
	}

	AMP_FATAL_ASSERT(font.m_vertexGrid.IsEmpty(), "Font meshes shouldn't be double-initialized!");

	// Create a vertex buffer where each vertex is a corner of a pixel. Vertices span the space from (0, 0) to (1, 1).
	for (size_t y = 0; y <= characterHeightPixels; ++y)
	{
		const float fY = static_cast<float>(y);
		for (size_t x = 0; x <= characterWidthPixels; ++x)
		{
			const float fX = static_cast<float>(x);

			Mesh::Vertex& vertex = font.m_vertexGrid.Emplace();
			vertex.m_x = fX / characterWidthPixels;
			vertex.m_y = fY / characterHeightPixels;
			vertex.m_z = 0.0f;
			vertex.m_abgr = 0xff0000ff;
		}
	}

	font.m_glyphVertexBufferHandle = bgfx::createVertexBuffer(
		bgfx::makeRef(&font.m_vertexGrid.Front(), font.m_vertexGrid.Size() * sizeof(Mesh::Vertex)),
		m_vertexDecl);

	// Create index buffers for each character.
	for (uint32_t y = 0; y < numRows; ++y)
	{
		for (uint32_t x = 0; x < numColumns; ++x)
		{
			const size_t characterIndex = ((numRows - y - 1) * numColumns) + x;
			Collection::Vector<uint16_t>& triangleIndexBuffer = font.m_glyphIndexBuffers[characterIndex];

			const uint32_t pixelXOffset = x * characterWidthPixels;
			const uint32_t pixelYOffset = y * characterHeightPixels;

			for (uint32_t pixelY = 0; pixelY < characterHeightPixels; ++pixelY)
			{
				for (uint32_t pixelX = 0; pixelX < characterWidthPixels; ++pixelX)
				{
					if (!image.GetValue(pixelX + pixelXOffset, pixelY + pixelYOffset))
					{
						continue;
					}

					const uint16_t i0 = static_cast<uint16_t>((pixelY * (characterWidthPixels + 1)) + pixelX);
					const uint16_t i1 = i0 + 1;
					const uint16_t i2 = static_cast<uint16_t>(((pixelY + 1) * (characterWidthPixels + 1)) + pixelX);
					const uint16_t i3 = i2 + 1;
					AMP_FATAL_ASSERT(i3 < font.m_vertexGrid.Size(), "Generated pixel corner index is out of bounds!");

					triangleIndexBuffer.Add(i0);
					triangleIndexBuffer.Add(i1);
					triangleIndexBuffer.Add(i2);

					triangleIndexBuffer.Add(i2);
					triangleIndexBuffer.Add(i1);
					triangleIndexBuffer.Add(i3);
				}
			}

			if (!triangleIndexBuffer.IsEmpty())
			{
				font.m_glyphIndexBufferHandles[characterIndex] = bgfx::createIndexBuffer(
					bgfx::makeRef(&triangleIndexBuffer.Front(), triangleIndexBuffer.Size() * sizeof(uint16_t)));
			}
		}
	}
}

void TextRenderer::SubmitCharacterQuad(bgfx::Encoder& encoder,
	const FontMeshDatum& font,
	const Math::Matrix4x4& uiTransform,
	const float characterWidth,
	const float characterHeight,
	const int x,
	const int y,
	const char c) const
{
	AMP_FATAL_ASSERT(bgfx::isValid(font.m_glyphVertexBufferHandle),
		"RenderCharacterQuad shouldn't be called with an uninitialized font!");

	const auto& indexBuffer = font.m_glyphIndexBufferHandles[c];
	if (!bgfx::isValid(indexBuffer))
	{
		return;
	}

	Math::Vector3 offset{ 0.0f, 0.0f, 0.0f };
	offset.x = x * characterWidth;
	offset.y = y * characterHeight;

	Math::Matrix4x4 characterTransform;
	characterTransform.SetTranslation(offset);
	characterTransform.SetScale(Math::Vector3(characterWidth, characterHeight, 0.0f));

	const Math::Matrix4x4 transform = uiTransform * characterTransform;

	encoder.setTransform(transform.GetData());
	encoder.setVertexBuffer(0, font.m_glyphVertexBufferHandle);
	encoder.setIndexBuffer(indexBuffer);
	encoder.setState(BGFX_STATE_DEFAULT);
	encoder.submit(k_uiViewID, m_program);
}
}
