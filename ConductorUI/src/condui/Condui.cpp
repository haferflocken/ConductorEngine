#include <condui/Condui.h>

#include <condui/ConduiEntityInfo.h>
#include <condui/TextDisplayComponent.h>
#include <condui/UITransfromComponent.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>

Condui::ConduiElement Condui::MakeTextDisplayElement(const char* const str)
{
	auto element = ConduiElement::Make<TextDisplayElement>();
	TextDisplayElement& textDisplayElement = element.Get<TextDisplayElement>();

	textDisplayElement.m_string = str;

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

ECS::Entity& Condui::CreateConduiEntity(
	const ECS::EntityInfoManager& entityInfoManager,
	ECS::EntityManager& entityManager,
	const ConduiElement& element)
{
	const Util::StringHash infoNameHash = GetEntityInfoNameHashFor(element);
	const ECS::EntityInfo& entityInfo = *entityInfoManager.FindEntityInfo(infoNameHash);

	ECS::Entity& entity = entityManager.CreateEntity(entityInfo);
	element.Match(
		[&](const TextDisplayElement& textDisplayElement)
		{
			TextDisplayComponent& textDisplayComponent = *entityManager.FindComponent<TextDisplayComponent>(entity);
			textDisplayComponent.m_string = textDisplayElement.m_string;
		},
		[&](const PanelElement& panelElement)
		{
			AMP_FATAL_ASSERT(panelElement.m_children.Size() == panelElement.m_childRelativeTransforms.Size(),
				"There is an expected 1-to-1 relationship between elements in a panel and their relative transforms.");
			for (size_t i = 0, iEnd = panelElement.m_children.Size(); i < iEnd; ++i)
			{
				const Math::Matrix4x4& transformFromParent = panelElement.m_childRelativeTransforms[i];
				const ConduiElement& childElement = panelElement.m_children[i];

				ECS::Entity& childEntity = CreateConduiEntity(entityInfoManager, entityManager, childElement);
				UITransformComponent& childTransformComponent =
					*entityManager.FindComponent<UITransformComponent>(childEntity);
				childTransformComponent.m_transformFromParent = transformFromParent;

				entityManager.SetParentEntity(childEntity, &entity);
			}
		});
	return entity;
}

ECS::Entity& Condui::CreateConduiRootEntity(
	const ECS::EntityInfoManager& entityInfoManager,
	ECS::EntityManager& entityManager,
	const ElementRoot& elementRoot)
{
	ECS::Entity& entity = CreateConduiEntity(entityInfoManager, entityManager, elementRoot.m_element);
	UITransformComponent& transformComponent = *entityManager.FindComponent<UITransformComponent>(entity);
	transformComponent.m_uiTransform = elementRoot.m_uiTransform;
	return entity;
}
