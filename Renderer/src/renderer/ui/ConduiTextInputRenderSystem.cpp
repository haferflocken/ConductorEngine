#include <renderer/ui/ConduiTextInputRenderSystem.h>

#include <mesh/StaticMesh.h>
#include <renderer/PrimitiveRenderer.h>
#include <renderer/ui/TextRenderer.h>
#include <renderer/ViewIDs.h>

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
		const auto& transformComponent = ecsGroup.Get<const Scene::SceneTransformComponent>();
		const auto& textComponent = ecsGroup.Get<const Condui::TextInputComponent>();

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
			textComponent.m_text.c_str(),
			textComponent.m_textHeight);

		PrimitiveRenderer::DrawQuad(
			*encoder,
			k_sceneViewID,
			transform,
			textComponent.m_width,
			textComponent.m_height,
			textComponent.m_backgroundColour);
	}

	bgfx::end(encoder);
}
}
