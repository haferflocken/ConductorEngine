#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <client/ConnectedHost.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(connectedHost)
	, m_gameData(gameData)
	, m_entityManager(gameData.GetComponentReflector())
{
	const Behave::BehaveContext context{ m_gameData.GetBehaviourTreeManager() };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));
}

void IslandGame::Client::IslandGameClient::Update()
{
	// TODO somehow stuff gets mirrored from the host??
	m_entityManager.Update();
}
