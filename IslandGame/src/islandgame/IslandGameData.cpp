#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>
#include <islandgame/IslandGameData.h>

#include <behave/ActorComponentFactory.h>
#include <behave/ActorComponentInfoFactory.h>
#include <behave/ActorInfoManager.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeManager.h>

IslandGame::IslandGameData::IslandGameData()
	: m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>())
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_actorComponentInfoFactory(Mem::MakeUnique<Behave::ActorComponentInfoFactory>())
	, m_actorInfoManager(Mem::MakeUnique<Behave::ActorInfoManager>(*m_actorComponentInfoFactory, *m_behaviourTreeManager))
	, m_actorComponentFactory(Mem::MakeUnique<Behave::ActorComponentFactory>())
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
