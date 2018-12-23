#include <renderer/ui/ConduiTextDisplaySystem.h>

#include <renderer/ViewIDs.h>

namespace Renderer::UI
{
TextDisplaySystem::TextDisplaySystem(uint16_t widthPixels, uint16_t heightPixels)
	: SystemTempl()
	, m_widthPixels(widthPixels)
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

TextDisplaySystem::~TextDisplaySystem()
{
	//bgfx::destroy(m_program);
}

void TextDisplaySystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	// Create font meshes for any code page images that finished loading.
	for (auto& entry : m_fontMeshData)
	{
		const Asset::AssetHandle<Image::Pixel1Image>& handle = entry.first;
		FontMeshDatum& font = entry.second;

		const Image::Pixel1Image* const codePage = handle.TryGetAsset();
		if (codePage == nullptr)
		{
			continue;
		}

		if (!bgfx::isValid(font.m_glyphVertexBufferHandle))
		{
			CreateFontMeshFromImage(*codePage, font);
		}
	}

	// Render each text display entity using its font mesh.
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Condui::UITransformComponent>();
		const auto& textComponent = ecsGroup.Get<const Condui::TextDisplayComponent>();

		const auto datumIter = m_fontMeshData.Find(textComponent.m_codePage);
		if (datumIter == m_fontMeshData.end())
		{
			continue;
		}

		const FontMeshDatum& font = datumIter->second;
		if (!bgfx::isValid(font.m_glyphVertexBufferHandle))
		{
			continue;
		}

		const float characterWidth = (textComponent.m_fontScale * textComponent.m_characterWidthPixels) / m_widthPixels;
		const float characterHeight = (textComponent.m_fontScale * textComponent.m_characterHeightPixels) / m_heightPixels;

		int x = 0;
		int y = -1; // We begin at -1 because characters render from the bottom up.
		char previous = '#';
		
		for (const auto& c : textComponent.m_string)
		{
			switch (c)
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
				RenderCharacterQuad(*encoder,
					font,
					transformComponent.m_uiTransform,
					characterWidth,
					characterHeight,
					x,
					y,
					c);

				x += 1;
				break;
			}
			}
			previous = c;
		}
	}
}

void TextDisplaySystem::NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group)
{
	// Create an entry in the font mesh data for the entity's code page handle.
	const auto& textDisplayComponent = group.Get<const Condui::TextDisplayComponent>();
	FontMeshDatum& font = m_fontMeshData[textDisplayComponent.m_codePage];
	font.m_characterWidthPixels = textDisplayComponent.m_characterWidthPixels;
	font.m_characterHeightPixels = textDisplayComponent.m_characterHeightPixels;
}

void TextDisplaySystem::NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group)
{
}

TextDisplaySystem::FontMeshDatum::FontMeshDatum()
{
	std::fill(m_glyphIndexBufferHandles.begin(), m_glyphIndexBufferHandles.end(),
		bgfx::IndexBufferHandle(BGFX_INVALID_HANDLE));
}

TextDisplaySystem::FontMeshDatum::~FontMeshDatum()
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

void TextDisplaySystem::CreateFontMeshFromImage(const Image::Pixel1Image& image,
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

void TextDisplaySystem::RenderCharacterQuad(bgfx::Encoder& encoder,
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
