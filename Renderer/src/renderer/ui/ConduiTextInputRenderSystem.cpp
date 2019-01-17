#include <renderer/ui/ConduiTextInputRenderSystem.h>

#include <image/Colour.h>
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

		const Math::Matrix4x4& transform = transformComponent.m_matrix;

		Math::Vector3 topPosition = transform.GetTranslation();
		topPosition.y += textComponent.m_yScale;

		Math::Matrix4x4 textTransform = transform;
		textTransform.SetTranslation(topPosition);

		m_textRenderer.SubmitText(*encoder,
			k_sceneViewID,
			textTransform,
			Image::ColoursARBG::k_black,
			textComponent.m_codePage,
			textComponent.m_text.c_str(),
			textComponent.m_fontScale);

		PrimitiveRenderer::DrawQuad(*encoder, k_sceneViewID, transform, Image::ColoursARBG::k_cyan);
	}

	bgfx::end(encoder);
}
}
