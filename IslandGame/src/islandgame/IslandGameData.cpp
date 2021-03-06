#include <islandgame/IslandGameData.h>

#include <islandgame/components/IslanderComponent.h>

#include <behave/ast/Interpreter.h>
#include <ecs/ComponentReflector.h>

IslandGame::IslandGameData::IslandGameData(const File::Path& dataDirectory, const File::Path& userDirectory,
	Asset::AssetManager& assetManager)
	: IGameData(dataDirectory, userDirectory, assetManager)
{
	using namespace IslandGame::Components;

	// TODO bind functions in the interpreter

	m_componentReflector->RegisterComponentType<IslanderComponent>();
}

IslandGame::IslandGameData::~IslandGameData()
{}

