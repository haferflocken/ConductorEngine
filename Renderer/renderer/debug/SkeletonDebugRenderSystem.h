#pragma once

#include <asset/AssetHandle.h>
#include <ecs/System.h>
#include <mesh/MeshComponent.h>

namespace Image { class Pixel1Image; }
namespace Renderer::UI { class TextRenderer; }

namespace Renderer::Debug
{
/**
 * Draws the skeletons of entities.
 */
class SkeletonDebugRenderSystem final : public ECS::SystemTempl<
	Util::TypeList<Mesh::MeshComponent>,
	Util::TypeList<>>
{
public:
	SkeletonDebugRenderSystem(
		UI::TextRenderer& textRenderer,
		const Asset::AssetHandle<Image::Pixel1Image>& codePage,
		const uint16_t characterWidthPixels,
		const uint16_t characterHeightPixels)
		: m_textRenderer(textRenderer)
		, m_codePage(codePage)
		, m_characterWidthPixels(characterWidthPixels)
		, m_characterHeightPixels(characterHeightPixels)
	{}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;

private:
	UI::TextRenderer& m_textRenderer;
	Asset::AssetHandle<Image::Pixel1Image> m_codePage;
	uint16_t m_characterWidthPixels;
	uint16_t m_characterHeightPixels;
};
}
