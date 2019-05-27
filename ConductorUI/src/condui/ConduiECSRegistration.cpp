#include <condui/ConduiECSRegistration.h>

#include <condui/components/StackingPanelComponent.h>
#include <condui/components/TextDisplayComponent.h>
#include <condui/components/TextInputComponent.h>
#include <condui/systems/StackingPanelSystem.h>
#include <condui/systems/TextDisplayUpdateSystem.h>
#include <condui/systems/TextInputSystem.h>

#include <dev/Profiler.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityManager.h>

#include <profilerui/ProfilerRootComponent.h>
#include <profilerui/ProfilerRootSystem.h>
#include <profilerui/ProfilerThreadComponent.h>
#include <profilerui/ProfilerThreadSystem.h>

void Condui::RegisterComponentTypes(ECS::ComponentReflector& componentReflector)
{
	componentReflector.RegisterComponentType<ProfilerUI::ProfilerRootComponent>();
	componentReflector.RegisterComponentType<ProfilerUI::ProfilerThreadComponent>();
	componentReflector.RegisterComponentType<StackingPanelComponent>();
	componentReflector.RegisterComponentType<TextDisplayComponent>();
	componentReflector.RegisterComponentType<TextInputComponent>();
}

void Condui::RegisterSystems(ECS::EntityManager& entityManager,
	const Math::Frustum& sceneViewFrustum,
	Input::CallbackRegistry& callbackRegistry)
{
#if AMP_PROFILING_ENABLED == 1
	entityManager.RegisterSystem(Mem::MakeUnique<ProfilerUI::ProfilerRootSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<ProfilerUI::ProfilerThreadSystem>());
#endif
	entityManager.RegisterSystem(Mem::MakeUnique<StackingPanelSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<TextDisplayUpdateSystem>());
	entityManager.RegisterSystem(Mem::MakeUnique<TextInputSystem>(sceneViewFrustum, callbackRegistry));
}
