#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <ecs/EntityInfoManager.h>
#include <scene/UnboundedScene.h>

namespace Internal_IslandGameHost
{
}

IslandGame::Host::IslandGameHost::IslandGameHost(const IslandGameData& gameData)
	: IHost(gameData.GetAssetManager(), gameData.GetComponentReflector())
	, m_gameData(gameData)
{
	const Behave::BehaveContext context{
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager };
	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(context));
	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::UnboundedScene>(m_gameData.GetEntityInfoManager()));

	m_entityManager.CreateEntity(*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("islander.json")));
}

IslandGame::Host::IslandGameHost::~IslandGameHost()
{
}

void IslandGame::Host::IslandGameHost::Update()
{
	m_entityManager.Update();
}
