#pragma once

#include <condui/TextDisplayComponent.h>
#include <condui/UITransformComponent.h>
#include <ecs/System.h>

namespace Renderer::UI
{
class TextRenderer;

/**
 * Makes entities visible by rendering their Condui::TextDisplayComponent.
 */
class TextDisplayRenderSystem final : public ECS::SystemTempl<
	Util::TypeList<Condui::UITransformComponent, Condui::TextDisplayComponent>,
	Util::TypeList<>>
{
public:
	explicit TextDisplayRenderSystem(TextRenderer& textRenderer);
	~TextDisplayRenderSystem();

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	TextRenderer& m_textRenderer;
};
}
