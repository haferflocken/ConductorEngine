#pragma once

#include <condui/components/TextDisplayComponent.h>
#include <ecs/System.h>
#include <scene/SceneTransformComponent.h>

namespace Renderer::UI
{
class TextRenderer;

/**
 * Makes entities visible by rendering their Condui::TextDisplayComponent.
 */
class TextDisplayRenderSystem final : public ECS::SystemTempl<
	Util::TypeList<Scene::SceneTransformComponent, Condui::TextDisplayComponent>,
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
