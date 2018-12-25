#pragma once

#include <condui/TextInputComponent.h>
#include <condui/UITransformComponent.h>
#include <ecs/System.h>

namespace Renderer::UI
{
class TextRenderer;

/**
 * Makes entities visible by rendering their Condui::TextInputComponent.
 */
class TextInputRenderSystem final : public ECS::SystemTempl<
	Util::TypeList<Condui::UITransformComponent, Condui::TextInputComponent>,
	Util::TypeList<>>
{
public:
	explicit TextInputRenderSystem(TextRenderer& textRenderer);
	~TextInputRenderSystem();

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	TextRenderer& m_textRenderer;
};
}

