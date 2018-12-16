#include <islandgame/host/IslandGameHost.h>

#include <islandgame/IslandGameData.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeEvaluationSystem.h>
#include <ecs/EntityInfoManager.h>
#include <input/InputSystem.h>
#include <scene/RelativeTransformSystem.h>
#include <scene/SceneAnchorSystem.h>
#include <scene/UnboundedScene.h>

namespace Internal_IslandGameHost
{
constexpr const char* k_chunkSourceDirectory = "scenes/test_scene";
constexpr const char* k_chunkUserDirectory = "";
}

IslandGame::Host::IslandGameHost::IslandGameHost(const IslandGameData& gameData)
	: IHost(gameData.GetAssetManager(), gameData.GetComponentReflector())
	, m_gameData(gameData)
{
	using namespace Internal_IslandGameHost;

	m_entityManager.RegisterSystem(Mem::MakeUnique<Behave::BehaviourTreeEvaluationSystem>(Behave::BehaveContext{
		m_gameData.GetBehaviourTreeManager(),
		m_gameData.GetBehaveASTInterpreter(),
		m_entityManager}));

	Scene::UnboundedScene& scene = m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::UnboundedScene>(
		m_gameData.GetEntityInfoManager(),
		gameData.GetDataDirectory() / k_chunkSourceDirectory,
		gameData.GetUserDirectory() / k_chunkUserDirectory));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::SceneAnchorSystem>(scene));

	m_entityManager.RegisterSystem(Mem::MakeUnique<Scene::RelativeTransformSystem>());

	//m_entityManager.CreateEntity(*m_gameData.GetEntityInfoManager().FindEntityInfo(Util::CalcHash("player.json")));
}

IslandGame::Host::IslandGameHost::~IslandGameHost()
{
}

void IslandGame::Host::IslandGameHost::Update(const Unit::Time::Millisecond delta)
{
	m_entityManager.Update(delta);
}
