#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <client/ConnectedHost.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(gameData.GetComponentReflector(), connectedHost)
	, m_gameData(gameData)
{
	const Behave::BehaveContext context{ 
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveConditionASTInterpreter() };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));
}

void IslandGame::Client::IslandGameClient::Update()
{
	m_entityManager.Update();
}
