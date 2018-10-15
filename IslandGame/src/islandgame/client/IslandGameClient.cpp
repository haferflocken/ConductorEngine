#include <islandgame/client/IslandGameClient.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <client/ConnectedHost.h>
#include <ecs/EntityInfoManager.h>

IslandGame::Client::IslandGameClient::IslandGameClient(
	const IslandGameData& gameData, ::Client::ConnectedHost& connectedHost)
	: IClient(gameData.GetAssetManager(), gameData.GetComponentReflector(), connectedHost)
	, m_gameData(gameData)
{
	const Behave::BehaveContext context{ 
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));
}

void IslandGame::Client::IslandGameClient::Update(const Unit::Time::Millisecond delta)
{
	// TODO remove this
	static bool isCameraCreated = false;
	if (!isCameraCreated)
	{
		m_entityManager.CreateEntity(*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("islander.json")));
		m_entityManager.CreateEntity(*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("camera.json")));
		isCameraCreated = true;
	}

	m_entityManager.Update(delta);
}
