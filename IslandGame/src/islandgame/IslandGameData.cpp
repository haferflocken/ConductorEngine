#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>
#include <islandgame/IslandGameData.h>

#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeManager.h>
#include <behave/conditionast/Interpreter.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>

IslandGame::IslandGameData::IslandGameData()
	: m_behaveConditionASTInterpreter(Mem::MakeUnique<Behave::ConditionAST::Interpreter>())
	, m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>(*m_behaveConditionASTInterpreter))
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_componentInfoFactory(Mem::MakeUnique<ECS::ComponentInfoFactory>())
	, m_entityInfoManager(Mem::MakeUnique<ECS::EntityInfoManager>(*m_componentInfoFactory, *m_behaviourTreeManager))
	, m_componentReflector(Mem::MakeUnique<ECS::ComponentReflector>())
{
	using namespace IslandGame::Components;

	// TODO bind functions in the interpreter

	m_componentInfoFactory->RegisterFactoryFunction<IslanderComponentInfo>();
	m_componentReflector->RegisterComponentType<IslanderComponent>();
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
