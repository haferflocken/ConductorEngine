#include <islandgame/IslandGameData.h>

#include <islandgame/components/IslanderComponent.h>
#include <islandgame/components/IslanderComponentInfo.h>

#include <behave/ast/Interpreter.h>
#include <ecs/ComponentInfoFactory.h>
#include <ecs/ComponentReflector.h>

IslandGame::IslandGameData::IslandGameData()
	: IGameData()
{
	using namespace IslandGame::Components;

	// TODO bind functions in the interpreter

	m_componentInfoFactory->RegisterFactoryFunction<IslanderComponentInfo>();
	m_componentReflector->RegisterComponentType<IslanderComponent>();
}

IslandGame::IslandGameData::~IslandGameData()
{}

