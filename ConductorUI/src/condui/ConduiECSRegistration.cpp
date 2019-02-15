#include <condui/ConduiECSRegistration.h>

#include <condui/StackingPanelComponent.h>
#include <condui/StackingPanelSystem.h>
#include <condui/TextDisplayComponent.h>
#include <condui/TextInputComponent.h>
#include <condui/TextInputSystem.h>

#include <ecs/ComponentReflector.h>
#include <ecs/EntityManager.h>

void Condui::RegisterComponentTypes(ECS::ComponentReflector& componentReflector)
{
	componentReflector.RegisterComponentType<StackingPanelComponent>();
	componentReflector.RegisterComponentType<TextDisplayComponent>();
	componentReflector.RegisterComponentType<TextInputComponent>();
}

void Condui::RegisterSystems(ECS::EntityManager& entityManager,
	const Math::Frustum& sceneViewFrustum,
	Input::CallbackRegistry& callbackRegistry)
{
	entityManager.RegisterSystem(Mem::MakeUnique<StackingPanelSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<TextInputSystem>(sceneViewFrustum, callbackRegistry));
}
