#include <condui/EntityInspector.h>

#include <condui/ConduiInspector.h>

#include <ecs/Component.h>
#include <ecs/ComponentReflector.h>
#include <ecs/Entity.h>
#include <ecs/EntityManager.h>

Condui::ConduiElement Condui::MakeEntityInspector(
	const ECS::ComponentReflector& componentReflector,
	ECS::EntityManager& entityManager,
	ECS::Entity& subject,
	const float width,
	const float textHeight)
{
	// TODO(inspector) allow adding/removing components

	// Create inspectors for each component.
	Collection::Vector<ConduiElement> subelements;

	const Collection::Vector<ECS::ComponentID>& componentIDs = subject.GetComponentIDs();
	for (const auto& componentID : componentIDs)
	{
		ECS::Component* const component = entityManager.FindComponent(componentID);
		if (component != nullptr)
		{
			const ECS::ComponentType componentType = componentID.GetType();
			const Mem::InspectorInfoTypeHash typeHash = componentReflector.GetTypeHashOfComponent(componentType);
			const auto inspectorInfo = Mem::InspectorInfo::Find(typeHash);

			ConduiElement componentInspector =
				Condui::MakeInspectorElement(inspectorInfo, component, width, textHeight);

			subelements.Add(std::move(componentInspector));
		}
	}

	return Condui::MakeStackingPanelElement(width, std::move(subelements));
}
