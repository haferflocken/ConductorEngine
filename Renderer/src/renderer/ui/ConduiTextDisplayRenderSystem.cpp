#include <renderer/ui/ConduiTextDisplayRenderSystem.h>

#include <image/Colour.h>
#include <renderer/ui/TextRenderer.h>
#include <renderer/ViewIDs.h>

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
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		const auto& textComponent = ecsGroup.Get<const Condui::TextDisplayComponent>();

		m_textRenderer.RequestFont(
			textComponent.m_codePage, textComponent.m_characterWidthPixels, textComponent.m_characterHeightPixels);

		const Math::Matrix4x4& transform = transformComponent.m_modelToWorldMatrix;

		const Math::Matrix4x4 textToTopTransform = Math::Matrix4x4::MakeTranslation(0.0f, textComponent.m_height, 0.0f);
		const Math::Matrix4x4 textTransform = transform * textToTopTransform;

		m_textRenderer.SubmitText(*encoder,
			k_sceneViewID,
			textTransform,
			textComponent.m_textColour,
			textComponent.m_codePage,
			textComponent.m_string.c_str(),
			textComponent.m_textHeight);
	}

	bgfx::end(encoder);
}
}
