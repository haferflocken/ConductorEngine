#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>
#include <islandgame/IslandGameData.h>

#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeManager.h>
#include <ecs/ComponentFactory.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/EntityInfoManager.h>

IslandGame::IslandGameData::IslandGameData()
	: m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>())
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_componentInfoFactory(Mem::MakeUnique<ECS::ComponentInfoFactory>())
	, m_entityInfoManager(Mem::MakeUnique<ECS::EntityInfoManager>(*m_componentInfoFactory, *m_behaviourTreeManager))
	, m_componentFactory(Mem::MakeUnique<ECS::ComponentFactory>())
{
	using namespace IslandGame::Components;

	m_componentInfoFactory->RegisterFactoryFunction<IslanderComponentInfo>();
	m_componentFactory->RegisterComponentType<IslanderComponent>();
}

IslandGame::IslandGameData::~IslandGameData()
{}

void IslandGame::IslandGameData::LoadBehaviourTreesInDirectory(const File::Path& directory)
{
	m_behaviourTreeManager->LoadTreesInDirectory(directory);
}

void IslandGame::IslandGameData::LoadEntityInfosInDirectory(const File::Path& directory)
{
	m_entityInfoManager->LoadEntityInfosInDirectory(directory);
}
