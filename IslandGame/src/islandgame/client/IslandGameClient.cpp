#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <client/ConnectedHost.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(connectedHost)
	, m_gameData(gameData)
	, m_actorManager(gameData.GetActorComponentFactory())
{}

void IslandGame::Client::IslandGameClient::Update()
{
	// TODO somehow stuff gets mirrored from the host??
	const Behave::BehaveContext context{ m_gameData.GetBehaviourTreeManager() };
	m_actorManager.Update(context);
}
