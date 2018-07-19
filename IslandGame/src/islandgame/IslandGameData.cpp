#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>
#include <islandgame/IslandGameData.h>

#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeManager.h>
#include <ecs/ActorComponentFactory.h>
#include <ecs/ActorComponentInfoFactory.h>
#include <ecs/ActorInfoManager.h>

IslandGame::IslandGameData::IslandGameData()
	: m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>())
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_actorComponentInfoFactory(Mem::MakeUnique<ECS::ActorComponentInfoFactory>())
	, m_actorInfoManager(Mem::MakeUnique<ECS::ActorInfoManager>(*m_actorComponentInfoFactory, *m_behaviourTreeManager))
	, m_actorComponentFactory(Mem::MakeUnique<ECS::ActorComponentFactory>())
{
	using namespace IslandGame::Components;

	m_actorComponentInfoFactory->RegisterFactoryFunction<IslanderComponentInfo>();
	m_actorComponentFactory->RegisterComponentType<IslanderComponent>();
}

IslandGame::IslandGameData::~IslandGameData()
{}

void IslandGame::IslandGameData::LoadBehaviourTreesInDirectory(const File::Path& directory)
{
	m_behaviourTreeManager->LoadTreesInDirectory(directory);
}

void IslandGame::IslandGameData::LoadActorInfosInDirectory(const File::Path& directory)
{
	m_actorInfoManager->LoadActorInfosInDirectory(directory);
}
