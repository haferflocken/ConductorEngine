#include <condui/Condui.h>

#include <condui/ConduiECSRegistration.h>
#include <condui/TextDisplayComponent.h>
#include <condui/TextInputComponent.h>
#include <ecs/EntityManager.h>
#include <scene/SceneTransformComponent.h>

Condui::ConduiElement Condui::MakeTextDisplayElement(const char* const str, const float fontScale)
{
	auto element = ConduiElement::Make<TextDisplayElement>();
	TextDisplayElement& textDisplayElement = element.Get<TextDisplayElement>();

	textDisplayElement.m_string = str;
	textDisplayElement.m_fontScale = fontScale;

	return element;
}

Condui::ConduiElement Condui::MakeTextInputElement(
	const float xScale, const float yScale, TextInputElement::InputHandler&& inputHandler, const float fontScale)
{
	auto element = ConduiElement::Make<TextInputElement>();
	TextInputElement& textInputElement = element.Get<TextInputElement>();

	textInputElement.m_inputHandler = std::move(inputHandler);
	textInputElement.m_xScale = xScale;
	textInputElement.m_yScale = yScale;
	textInputElement.m_fontScale = fontScale;

	return element;
}

Condui::ConduiElement Condui::MakePanelElement(
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>>&& childrenWithRelativeTransforms)
{
	auto element = ConduiElement::Make<PanelElement>();
	PanelElement& panelElement = element.Get<PanelElement>();

	for (const auto& pair : childrenWithRelativeTransforms)
	{
		panelElement.m_childRelativeTransforms.Add(pair.first);
		panelElement.m_children.Add(std::move(pair.second));
	}

	return element;
}

Condui::ConduiElement Condui::MakeTextInputCommandElement(const float xScale,
	const float yScale,
	Collection::VectorMap<const char*, std::function<void(TextInputComponent&)>>&& commandMap,
	const float fontScale)
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
	return MakeTextInputElement(xScale, yScale, std::move(commandHandler), fontScale);
}

ECS::Entity& Condui::CreateConduiEntity(ECS::EntityManager& entityManager, ConduiElement&& element)
{
	const Util::StringHash infoNameHash = GetEntityInfoNameHashFor(element);
	const ECS::EntityInfo& entityInfo = *entityInfoManager.FindEntityInfo(infoNameHash);

	ECS::Entity& entity = entityManager.CreateEntity(entityInfo);
	element.Match(
		[&](TextDisplayElement& textDisplayElement)
		{
			auto& textDisplayComponent = *entityManager.FindComponent<TextDisplayComponent>(entity);
			textDisplayComponent.m_string = std::move(textDisplayElement.m_string);
			textDisplayComponent.m_fontScale = textDisplayElement.m_fontScale;
		},
		[&](TextInputElement& textInputElement)
		{
			auto& textInputComponent = *entityManager.FindComponent<TextInputComponent>(entity);

			if (textInputElement.m_inputHandler)
			{
				textInputComponent.m_inputHandler = std::move(textInputElement.m_inputHandler);
			}

			textInputComponent.m_xScale = textInputElement.m_xScale;
			textInputComponent.m_yScale = textInputElement.m_yScale;
			textInputComponent.m_fontScale = textInputElement.m_fontScale;

			auto& sceneTransformComponent = *entityManager.FindComponent<Scene::SceneTransformComponent>(entity);
			sceneTransformComponent.m_childToParentMatrix.SetScale(
				Math::Vector3(textInputElement.m_xScale, textInputElement.m_yScale, 1.0f));
		},
		[&](PanelElement& panelElement)
		{
			AMP_FATAL_ASSERT(panelElement.m_children.Size() == panelElement.m_childRelativeTransforms.Size(),
				"There is an expected 1-to-1 relationship between elements in a panel and their relative transforms.");
			for (size_t i = 0, iEnd = panelElement.m_children.Size(); i < iEnd; ++i)
			{
				const Math::Matrix4x4& transformFromParent = panelElement.m_childRelativeTransforms[i];
				ConduiElement& childElement = panelElement.m_children[i];

				ECS::Entity& childEntity = CreateConduiEntity(entityManager, std::move(childElement));
				auto& childTransformComponent =
					*entityManager.FindComponent<Scene::SceneTransformComponent>(childEntity);
				childTransformComponent.m_childToParentMatrix =
					transformFromParent * childTransformComponent.m_childToParentMatrix;

				entityManager.SetParentEntity(childEntity, &entity);
			}
		});
	return entity;
}

ECS::Entity& Condui::CreateConduiRootEntity(ECS::EntityManager& entityManager, ElementRoot&& elementRoot)
{
	ECS::Entity& entity = CreateConduiEntity(entityManager, std::move(elementRoot.m_element));
	auto& transformComponent = *entityManager.FindComponent<Scene::SceneTransformComponent>(entity);
	transformComponent.m_modelToWorldMatrix = elementRoot.m_uiTransform;
	return entity;
}
