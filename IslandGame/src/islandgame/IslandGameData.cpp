#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>
#include <islandgame/IslandGameData.h>

#include <behave/ast/Interpreter.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourTreeManager.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>
#include <ecs/EntityInfoManager.h>

IslandGame::IslandGameData::IslandGameData()
	: m_componentReflector(Mem::MakeUnique<ECS::ComponentReflector>())
	, m_behaveASTInterpreter(Mem::MakeUnique<Behave::AST::Interpreter>(*m_componentReflector))
	, m_behaviourNodeFactory(Mem::MakeUnique<Behave::BehaviourNodeFactory>(*m_behaveASTInterpreter))
	, m_behaviourTreeManager(Mem::MakeUnique<Behave::BehaviourTreeManager>(*m_behaviourNodeFactory))
	, m_componentInfoFactory(Mem::MakeUnique<ECS::ComponentInfoFactory>())
	, m_entityInfoManager(Mem::MakeUnique<ECS::EntityInfoManager>(*m_componentInfoFactory, *m_behaviourTreeManager))
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
