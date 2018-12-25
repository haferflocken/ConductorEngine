#include <renderer/ui/ConduiTextDisplayRenderSystem.h>

#include <renderer/ui/TextRenderer.h>

namespace Renderer::UI
{
TextDisplayRenderSystem::TextDisplayRenderSystem(TextRenderer& textRenderer)
	: SystemTempl()
	, m_textRenderer(textRenderer)
{
}

TextDisplayRenderSystem::~TextDisplayRenderSystem()
{
}

void TextDisplayRenderSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	bgfx::Encoder* const encoder = bgfx::begin();
	if (encoder == nullptr)
	{
		return;
	}

	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& transformComponent = ecsGroup.Get<const Condui::UITransformComponent>();
		const auto& textComponent = ecsGroup.Get<const Condui::TextDisplayComponent>();

		m_textRenderer.RequestFont(
			textComponent.m_codePage, textComponent.m_characterWidthPixels, textComponent.m_characterHeightPixels);

		m_textRenderer.SubmitText(*encoder,
			transformComponent.m_uiTransform,
			textComponent.m_codePage,
			textComponent.m_string.c_str(),
			textComponent.m_fontScale);
	}
}
}
