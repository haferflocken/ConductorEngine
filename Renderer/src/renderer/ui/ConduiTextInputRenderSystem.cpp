#include <renderer/ui/ConduiTextInputRenderSystem.h>

#include <renderer/ui/TextRenderer.h>

namespace Renderer::UI
{
TextInputRenderSystem::TextInputRenderSystem(TextRenderer& textRenderer)
	: SystemTempl()
	, m_textRenderer(textRenderer)
{
}

TextInputRenderSystem::~TextInputRenderSystem()
{
}

void TextInputRenderSystem::Update(const Unit::Time::Millisecond delta,
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
		const auto& textComponent = ecsGroup.Get<const Condui::TextInputComponent>();

		m_textRenderer.RequestFont(
			textComponent.m_codePage, textComponent.m_characterWidthPixels, textComponent.m_characterHeightPixels);

		Math::Vector3 position = transformComponent.m_uiTransform.GetTranslation();
		position.y += textComponent.m_yScale;

		Math::Matrix4x4 transform = transformComponent.m_uiTransform;
		transform.SetTranslation(position);

		m_textRenderer.SubmitText(*encoder,
			transform,
			textComponent.m_codePage,
			textComponent.m_text.c_str(),
			textComponent.m_fontScale);
	}
}
}
