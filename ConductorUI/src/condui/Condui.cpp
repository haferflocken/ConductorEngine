#include <condui/Condui.h>

#include <condui/StackingPanelComponent.h>
#include <condui/TextDisplayComponent.h>
#include <condui/TextInputComponent.h>
#include <ecs/EntityManager.h>
#include <scene/SceneTransformComponent.h>

float Condui::ConduiElement::GetWidth() const
{
	float result = 0.0f;
	Match([&](const TextDisplayElement& element) { result = element.m_width; },
		[&](const TextInputElement& element) { result = element.m_width; },
		[&](const PanelElement& element) { result = element.m_width; },
		[&](const StackingPanelElement& element) { result = element.m_width; });
	return result;
}

float Condui::ConduiElement::GetHeight() const
{
	float result = 0.0f;
	Match([&](const TextDisplayElement& element) { result = element.m_height; },
		[&](const TextInputElement& element) { result = element.m_height; },
		[&](const PanelElement& element) { result = element.m_height; },
		[&](const StackingPanelElement& element) { result = element.m_height; });
	return result;
}

Condui::ConduiElement Condui::MakeTextDisplayElement(
	const float width, const float height, const char* const str, const float textHeight)
{
	return MakeTextDisplayElement(width, height, str, {}, textHeight);
}

Condui::ConduiElement Condui::MakeTextDisplayElement(
	const float width,
	const float height,
	const char* const str,
	TextDisplayElement::TextUpdateFunction&& updateFunction,
	const float textHeight)
{
	auto element = ConduiElement::Make<TextDisplayElement>();
	TextDisplayElement& textDisplayElement = element.Get<TextDisplayElement>();

	textDisplayElement.m_initialString = str;
	textDisplayElement.m_updateFunction = std::move(updateFunction);
	textDisplayElement.m_width = width;
	textDisplayElement.m_height = height;
	textDisplayElement.m_textHeight = textHeight;

	return element;
}

Condui::ConduiElement Condui::MakeTextInputElement(
	const float width,
	const float height,
	TextInputElement::InputHandler&& inputHandler,
	const float textHeight,
	const Image::ColourARGB backgroundColour)
{
	auto element = ConduiElement::Make<TextInputElement>();
	TextInputElement& textInputElement = element.Get<TextInputElement>();

	textInputElement.m_inputHandler = std::move(inputHandler);
	textInputElement.m_width = width;
	textInputElement.m_height = height;
	textInputElement.m_textHeight = textHeight;
	textInputElement.m_backgroundColour = backgroundColour;

	return element;
}

Condui::ConduiElement Condui::MakePanelElement(
	const float width,
	const float height,
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>>&& childrenWithRelativeTransforms)
{
	auto element = ConduiElement::Make<PanelElement>();
	PanelElement& panelElement = element.Get<PanelElement>();
	panelElement.m_width = width;
	panelElement.m_height = height;

	for (const auto& pair : childrenWithRelativeTransforms)
	{
		panelElement.m_childRelativeTransforms.Add(pair.first);
		panelElement.m_children.Add(std::move(pair.second));
	}

	return element;
}

Condui::ConduiElement Condui::MakeStackingPanelElement(
	const float width,
	Collection::Vector<ConduiElement>&& children)
{
	auto element = ConduiElement::Make<StackingPanelElement>();
	StackingPanelElement& stackingPanelElement = element.Get<StackingPanelElement>();
	stackingPanelElement.m_width = width;
	stackingPanelElement.m_height = 0.0f;
	stackingPanelElement.m_children = std::move(children);

	for (const auto& childElement : stackingPanelElement.m_children)
	{
		stackingPanelElement.m_height += childElement.GetHeight();
	}

	return element;
}

Condui::ConduiElement Condui::MakeTextInputCommandElement(
	const float width,
	const float height,
	Collection::VectorMap<const char*, std::function<void(TextInputComponent&)>>&& commandMap,
	const Image::ColourARGB backgroundColour)
{
	TextInputElement::InputHandler commandHandler =
		[commandsAndHandlers = std::move(commandMap)](TextInputComponent& component, const char* text) mutable
	{
		if (strcmp(text, "\r") != 0)
		{
			Condui::TextInputComponent::DefaultInputHandler(component, text);
			return;
		}

		for (const auto& entry : commandsAndHandlers)
		{
			if (strcmp(component.m_text.c_str(), entry.first) == 0)
			{
				entry.second(component);
				break;
			}
		}
		component.m_text.clear();
	};
	return MakeTextInputElement(width, height, std::move(commandHandler), height, backgroundColour);
}

ECS::Entity& Condui::CreateConduiEntity(
	ECS::EntityManager& entityManager, ConduiElement&& element, const FontInfo* font)
{
	ECS::Entity* entity = nullptr;
	element.Match(
		[&](TextDisplayElement& textDisplayElement)
		{
			const auto componentTypes = { TextDisplayComponent::k_type, Scene::SceneTransformComponent::k_type };
			entity = &entityManager.CreateEntityWithComponents(
				{ componentTypes.begin(), componentTypes.size() },
				ECS::EntityFlags::None,
				ECS::EntityLayers::k_conduiLayer);

			auto& textDisplayComponent = *entityManager.FindComponent<TextDisplayComponent>(*entity);

			textDisplayComponent.m_string = std::move(textDisplayElement.m_initialString);
			textDisplayComponent.m_stringUpdateFunction = std::move(textDisplayElement.m_updateFunction);

			if (font != nullptr)
			{
				textDisplayComponent.m_codePage = font->m_codePage;
				textDisplayComponent.m_characterWidthPixels = font->m_characterWidthPixels;
				textDisplayComponent.m_characterHeightPixels = font->m_characterHeightPixels;
				textDisplayComponent.m_textColour = font->m_textColour;
			}

			textDisplayComponent.m_width = textDisplayElement.m_width;
			textDisplayComponent.m_height = textDisplayElement.m_height;
			textDisplayComponent.m_textHeight = textDisplayElement.m_textHeight;
		},
		[&](TextInputElement& textInputElement)
		{
			const auto componentTypes = { TextInputComponent::k_type, Scene::SceneTransformComponent::k_type };
			entity = &entityManager.CreateEntityWithComponents(
				{ componentTypes.begin(), componentTypes.size() },
				ECS::EntityFlags::None,
				ECS::EntityLayers::k_conduiLayer);

			auto& textInputComponent = *entityManager.FindComponent<TextInputComponent>(*entity);

			if (textInputElement.m_inputHandler)
			{
				textInputComponent.m_inputHandler = std::move(textInputElement.m_inputHandler);
			}

			if (font != nullptr)
			{
				textInputComponent.m_codePage = font->m_codePage;
				textInputComponent.m_characterWidthPixels = font->m_characterWidthPixels;
				textInputComponent.m_characterHeightPixels = font->m_characterHeightPixels;
				textInputComponent.m_textColour = font->m_textColour;
			}

			textInputComponent.m_width = textInputElement.m_width;
			textInputComponent.m_height = textInputElement.m_height;
			textInputComponent.m_textHeight = textInputElement.m_textHeight;
			textInputComponent.m_backgroundColour = textInputElement.m_backgroundColour;
		},
		[&](PanelElement& panelElement)
		{
			AMP_FATAL_ASSERT(panelElement.m_children.Size() == panelElement.m_childRelativeTransforms.Size(),
				"There is an expected 1-to-1 relationship between elements in a panel and their relative transforms.");

			const auto componentTypes = { Scene::SceneTransformComponent::k_type };
			entity = &entityManager.CreateEntityWithComponents(
				{ componentTypes.begin(), componentTypes.size() },
				ECS::EntityFlags::None,
				ECS::EntityLayers::k_conduiLayer);

			for (size_t i = 0, iEnd = panelElement.m_children.Size(); i < iEnd; ++i)
			{
				const Math::Matrix4x4& transformFromParent = panelElement.m_childRelativeTransforms[i];
				ConduiElement& childElement = panelElement.m_children[i];

				ECS::Entity& childEntity = CreateConduiEntity(entityManager, std::move(childElement), font);
				auto& childTransformComponent =
					*entityManager.FindComponent<Scene::SceneTransformComponent>(childEntity);
				childTransformComponent.m_childToParentMatrix =
					transformFromParent * childTransformComponent.m_childToParentMatrix;

				entityManager.SetParentEntity(childEntity, entity);
			}
		},
		[&](StackingPanelElement& stackingPanelElement)
		{
			const auto componentTypes = { Scene::SceneTransformComponent::k_type, StackingPanelComponent::k_type };
			entity = &entityManager.CreateEntityWithComponents(
				{ componentTypes.begin(), componentTypes.size() },
				ECS::EntityFlags::None,
				ECS::EntityLayers::k_conduiLayer);

			float verticalOffset = 0.0f;
			for (auto& childElement : stackingPanelElement.m_children)
			{
				const auto transformFromParent = Math::Matrix4x4::MakeTranslation(0.0f, verticalOffset, 0.0f);
				verticalOffset -= childElement.GetHeight();

				ECS::Entity& childEntity = CreateConduiEntity(entityManager, std::move(childElement), font);
				auto& childTransformComponent =
					*entityManager.FindComponent<Scene::SceneTransformComponent>(childEntity);
				childTransformComponent.m_childToParentMatrix =
					transformFromParent * childTransformComponent.m_childToParentMatrix;

				entityManager.SetParentEntity(childEntity, entity);
			}
		});
	return *entity;
}
