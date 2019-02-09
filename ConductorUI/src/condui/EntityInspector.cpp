#include <condui/EntityInspector.h>

#include <condui/ConduiInspector.h>

#include <ecs/Component.h>
#include <ecs/ComponentReflector.h>
#include <ecs/Entity.h>
#include <ecs/EntityManager.h>

namespace Internal_EntityInspector
{
Condui::ConduiElement MakeComponentInspector(
	const ECS::ComponentReflector& componentReflector,
	const ECS::ComponentType componentType,
	ECS::Component& subject,
	const float width,
	const float textHeight,
	float& outHeight)
{
	const Mem::InspectorInfoTypeHash typeHash = componentReflector.GetTypeHashOfComponent(componentType);
	const auto inspectorInfo = Mem::InspectorInfo::Find(typeHash);

	Condui::ConduiElement inspectorElement = Condui::MakeInspectorElement(inspectorInfo, &subject, width, textHeight);

	const Condui::PanelElement& inspectorPanel = inspectorElement.Get<Condui::PanelElement>();
	outHeight = inspectorPanel.m_height;

	return inspectorElement;
}
}

Condui::ConduiElement Condui::MakeEntityInspector(
	const ECS::ComponentReflector& componentReflector,
	ECS::EntityManager& entityManager,
	ECS::Entity& subject,
	const float width,
	const float textHeight)
{
	using namespace Internal_EntityInspector;

	// TODO(inspector) allow adding/removing components

	// Create inspectors for each component.
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>> subelements;
	float verticalOffset = 0.0f;
	
	const Collection::Vector<ECS::ComponentID>& componentIDs = subject.GetComponentIDs();
	for (const auto& componentID : componentIDs)
	{
		ECS::Component* const component = entityManager.FindComponent(componentID);
		if (component != nullptr)
		{
			float componentInspectorHeight;
			ConduiElement componentInspector = MakeComponentInspector(
				componentReflector, componentID.GetType(), *component, width, textHeight, componentInspectorHeight);

			Math::Matrix4x4 transform;
			transform.SetTranslation(0.0f, verticalOffset, 0.0f);
			verticalOffset -= componentInspectorHeight;
			subelements.Emplace(transform, std::move(componentInspector));
		}
	}

	return Condui::MakePanelElement(width, -verticalOffset, std::move(subelements));
}
