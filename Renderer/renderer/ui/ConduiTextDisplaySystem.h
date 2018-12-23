#pragma once

#include <collection/VectorMap.h>
#include <condui/TextDisplayComponent.h>
#include <condui/UITransformComponent.h>
#include <ecs/EntityID.h>
#include <ecs/System.h>
#include <image/Pixel1Image.h>
#include <mesh/Vertex.h>

#include <bgfx/bgfx.h>

#include <array>

namespace Renderer::UI
{
/**
 * Makes entities visible by rendering their Condui::TextDisplayComponent.
 */
class TextDisplaySystem final : public ECS::SystemTempl<
	Util::TypeList<Condui::UITransformComponent, Condui::TextDisplayComponent>,
	Util::TypeList<>,
	ECS::SystemBindingType::Extended>
{
public:
	TextDisplaySystem(uint16_t widthPixels, uint16_t heightPixels);
	~TextDisplaySystem();

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

	void NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group);
	void NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group);

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

	void RenderCharacterQuad(bgfx::Encoder& encoder,
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
